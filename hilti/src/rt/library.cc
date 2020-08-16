// Copyright (c) 2020 by the Zeek Project. See LICENSE for details.

#include "hilti/rt/library.h"

#include <dlfcn.h>

#include <utility>

#include <hilti/rt/autogen/version.h>
#include <hilti/rt/exception.h>
#include <hilti/rt/fmt.h>
#include <hilti/rt/json.h>
#include <hilti/rt/logging.h>

using namespace hilti::rt;

std::string hilti::rt::library::Version::toJSON() const {
    return nlohmann::json{{"magic", magic},
                          {"hilti_version", hilti_version},
                          {"created", created},
                          {"debug", debug},
                          {"optimize", optimize}}
        .dump();
}

hilti::rt::Result<hilti::rt::library::Version> hilti::rt::library::Version::fromJSON(const std::string& json) {
    Version version;

    try {
        auto j = nlohmann::json::parse(json);
        j.at("magic").get_to(version.magic);
        j.at("hilti_version").get_to(version.hilti_version);
        j.at("created").get_to(version.created);
        j.at("debug").get_to(version.debug);
        j.at("optimize").get_to(version.optimize);
    } catch ( const nlohmann::json::exception& e ) {
        return result::Error(e.what());
    }

    return std::move(version);
}

void hilti::rt::library::Version::checkCompatibility() const {
    if ( hilti_version != PROJECT_VERSION_NUMBER )
        warning(fmt("module %s was compiled with HILTI version %d, but using HILTI version %d", path.filename(),
                    hilti_version, PROJECT_VERSION_NUMBER));

    if ( hilti::rt::isDebugVersion() && optimize )
        warning(
            fmt("module %s was compiled with optimizations, but running with HILTI debug version; performance will be "
                "affected",
                path.filename()));
}

hilti::rt::Library::Library(const std::filesystem::path& path) : _orig_path(path) {
    if ( ! std::filesystem::exists(path) )
        throw EnvironmentError(fmt("no such library: %s", path));

    auto path_ = createTemporaryFile(path.filename());
    if ( ! path_ )
        throw EnvironmentError(fmt("could not add library %s: %s", path, path_.error()));

    std::error_code ec;
    std::filesystem::copy(path, *path_, std::filesystem::copy_options::overwrite_existing, ec);
    if ( ec )
        throw EnvironmentError(fmt("could not store library %s at %s: %s", path, *path_, ec.message()));

    _path = std::filesystem::absolute(std::move(*path_));
}

hilti::rt::Library::~Library() {
    std::error_code ec;
    std::filesystem::remove(_path, ec);

    if ( ec )
        hilti::rt::warning(fmt("could not remove library %s from store: %s", _path, ec.message()));
}

hilti::rt::Result<hilti::rt::library::Version> hilti::rt::Library::open() const {
    constexpr auto mode = RTLD_LAZY | RTLD_GLOBAL;

    if ( ! _handle ) {
        void* handle = ::dlopen(_path.c_str(), mode);

        if ( ! ::dlopen(_path.c_str(), mode) )
            return result::Error(fmt("failed to load library %s: %s", _path, dlerror()));

        _handle = handle;
    }

    auto version_string = reinterpret_cast<const char**>(::dlsym(_handle, "__hlto_library_version"));
    if ( ! version_string )
        return result::Error("no version information");

    auto version = library::Version::fromJSON(*version_string);
    if ( ! version )
        return result::Error(fmt("broken version information (%s)", version.error()));

    // Check version. We only warn for now, don't abort.
    if ( version->magic != "v1" )
        result::Error(fmt("unknown HLTO version '%s'", version->magic));

    version->path = _orig_path;
    version->checkCompatibility();

    return version;
}

hilti::rt::Result<void*> hilti::rt::Library::symbol(std::string_view name) const {
    if ( ! _handle )
        return result::Error(fmt("library %s has not been opened", _path));

    auto* symbol = ::dlsym(_handle, name.data());
    // auto* symbol = ::dlsym(RTLD_SELF, name.data());

    if ( symbol == nullptr )
        return result::Error(fmt("symbol '%s' not found", name));

    return symbol;
}

hilti::rt::Result<hilti::rt::Nothing> hilti::rt::Library::save(const std::filesystem::path& path) const {
    std::error_code ec;
    std::filesystem::copy(_path, path, std::filesystem::copy_options::overwrite_existing, ec);

    if ( ec )
        return result::Error(fmt("could not save library to %s: %s", path, ec.message()));

    return hilti::rt::Nothing();
}
