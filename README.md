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

- [Snap package](https://snapcraft.io/json-tui): ```sudo snap install json-tui```. /!\ File access is confined to your home.
- Deb package: [Available](https://github.com/ArthurSonzogni/json-tui/releases) in release page.
- RPM package: [Available](https://github.com/ArthurSonzogni/json-tui/releases) in release page.
- Arch linux package: [Available](https://aur.archlinux.org/packages/json-tui-bin/) as AUR package.
- Binaries: [Available](https://github.com/ArthurSonzogni/json-tui/releases) in release page.

Authors:
--------
- Arthur Sonzogni
- *You?* (PR are welcomed)

Similar projects:
----------------
- https://github.com/antonmedv/fx
- https://github.com/skanehira/tson
- https://github.com/cielong/pyfx

[FTXUI]:https://github.com/ArthurSonzogni/FTXUI
