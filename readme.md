# classical rhythm game demo/playground

My goal is recreating something resembling core gameplay of something like `PaRappa the Rapper` or `Yakuza` karaoke minigame.

## Building

Intended building process requires cmake.

You'll also, of course, need any semi-modern c compiler and some generator (make, Ninja, etc.).

On linux, you'll also need to install [raylib dependencies](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux#install-required-libraries).

1. Clone repo with:
```bash
git clone --recurse-submodules https://github.com/mtrafisz/rhythm-demo.git
```
2. Run cmake:
```bash
cd rhythm-demo
cmake -S . -B build
```
3. Build using make
```bash
cd build
make # or Your favourite generator
```
Or
```
cmake --build build
```

## Todo
- [ ] Representing maps/songs in file and conversion of these to Array of NoteWindow

## License

See [LICENSE](LICENSE)
