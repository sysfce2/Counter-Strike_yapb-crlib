# crlib

crlib - is a small utility library for the YaPB bot and tools.

When initially bot was written as PODbot in 2000 year, there was awful support for STL in different compilers, so it was decision to not use STL inside bot project.

This is not true anymore, so this library will be deprecated as much as possible while converting the bot code to STL. However this is highly unlikely in near future. So this "wheel" will be here for some time.

# why the hell you still updating it in 2023?!
Well, i'm too lazy to rewrite code with STL in first place, second, the library allow's me to use limited subset on modern C++ and not link to the stdc++ nor static link with it. HLDS servers are often (more than often!) runs outdated on distros, and this allows me to compile code with newest compilers without breaking compatibility with ancient distros.