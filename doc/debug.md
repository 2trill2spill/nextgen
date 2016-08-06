
# Debug


## Sanitizers

You can enable sanitizer with SANITIZE_ADDRESS, SANITIZE_MEMORY, SANITIZE_THREAD or SANITIZE_UNDEFINED options in your CMake configuration. You can do this by passing e.g. -DSANITIZE_ADDRESS=On on your command line or with your graphical interface.

If sanitizers are supported by your compiler, the specified targets will be build with sanitizer support. If your compiler has no sanitizing capabilities (I asume intel compiler doesn't) you'll get a warning but CMake will continue processing and sanitizing will simply just be ignored.