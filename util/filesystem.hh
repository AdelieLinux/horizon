#if defined(FS_IS_BOOST)
#   include <boost/filesystem.hpp>
    namespace fs = boost::filesystem;
    using boost::system::error_code;
#   define fs_overwrite fs::copy_option::overwrite_if_exists
#   define rwxr_xr_x fs::perms::owner_all | fs::perms::group_read | fs::perms::group_exe | fs::perms::others_read | fs::perms::others_exe
#elif defined(FS_IS_STDCXX)
#   include <filesystem>
    namespace fs = std::filesystem;
    using std::error_code;
#   define fs_overwrite fs::copy_options::overwrite_existing
#   define rwxr_xr_x fs::perms::owner_all | fs::perms::group_read | fs::perms::group_exec | fs::perms::others_read | fs::perms::others_exec
#else
#   error Missing <filesystem> implementation.
#endif
