# f2http - File to HTTP
A simple interactive ad hoc http server

Need to make a specific file downloadable on a linux server so another server or tool can pick it up for download?

Let's say for example you need to present a .iso file to some OOM management tool as http source

```bash
f2http my_installer.iso 80
```

Just Control+C to stop the server.

# Build and install
It builds to a debian package by default.

```bash
make
sudo apt install $(ls ./f2http*.deb)
```

If you are not on a debian/ubuntu based distribution, use the following:
```bash
make
sudo make install
```