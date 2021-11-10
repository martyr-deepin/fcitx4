# Fcitx 4

Fcitx 4 is under maintainence Mode now, so no new issue and PR should be created in [fcitx/fcitx](https://github.com/fcitx/fcitx).

Please give [fcitx 5](https://github.com/fcitx/fcitx5) a try if possible. 

But if you want to improve fcitx 4 for learning purposes, you can create pr in [linuxdeepin/fcitx](https://github.com/linuxdeepin/fcitx).

## Description

 Fcitx is an input method framework with extension support, which provides an interface for entering characters of different scripts in applications using a variety of mapping systems.
 
 It offers a pleasant and modern experience, with intuitive graphical configuration tools and customizable skins and mapping tables. It is highly modularized and extensible, with GTK+ 2/3 and Qt 4/5 IM Modules, support for UIs based on Fbterm, pure Xlib, GTK+, or KDE, and a developer-friendly API.
 
 This metapackage pulls in a set of components recommended for most desktop users.

## Wiki
[https://fcitx-im.org/]( https://fcitx-im.org/)

## Installation

```bash
cmake ..  -DCMAKE_INSTALL_PREFIX=/usr
sudo make install
```

### Deepin System

```
sudo apt build-dep fcitx
sudo apt source fcitx
cd fcitx
mkair build
cmake ..  -DCMAKE_INSTALL_PREFIX=/usr
sudo make install
```

### Build deb package
```
sed -i 's/fcitx (/fcitx (1:/g' ./debian/changelog
sudo dpkg-buildpackage -b
```

## Contributing

Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update tests as appropriate.

## Discussions

  - [fcitx [at] googlegroups.com](https://groups.google.com/g/fcitx)
  - [https://github.com/fcitx/fcitx5/discussions](https://github.com/fcitx/fcitx5/discussions)
  - Telegram: https://fcitx-im.org/telegram/captcha.html
  - IRC: [#fcitx [at] libera.chat](https://web.libera.chat/?channels=#fcitx)

## Bug report

https://github.com/linuxdeepin/fcitx/issues

You can always report any fcitx 4 issue here, it might be transfer to other repos later.

## License

[LGPL-2.1+](https://spdx.org/licenses/LGPL-2.1-or-later.html)