json-tui
========

![Video](demo.webp)

Features
--------
- **Interactive**: Use keyboard or mouse to scroll/expand objects.
- **Colors**
- **Responsive**: Adapt to the terminal dimensions. Very long text values are
  wrapped on several lines.
- The output is displayed inline with the previous commands. Meaning you can
  still see the json after leaving json-tui.
- *(Vim users): Also support `j`/`k` for navigation.*

Features for developers
-----------------------
- **simple**: Only 300 line of C++ only. Depends on [FTXUI].
- No dependencies to install. Build simply using CMake.

Build:
------
```bash
mkdir build; cd build
cmake ..
make
sudo make install
```

Package
--------

- Snap package: To be added.
- Deb package: To be added.
- RPM package: To be added.
- Arch linux package: To be added.
- Binaries: To be added.

Authors:
--------
- Arthur Sonzogni
- *You?* (PR are welcomed)

[FTXUI]:https://github.com/ArthurSonzogni/FTXUI
