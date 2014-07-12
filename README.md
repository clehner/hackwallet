# HackWallet

Bitcoin wallet for GTK that lives in a tray menu. Inspired by MacWallet.

Currently this is just a UI demonstration.

## License

Copyright (C) 2014 Charles Lehner

HackWallet is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

HackWallet is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with HackWallet.  If not, see <http://www.gnu.org/licenses/>.


## Notes

### Bitcoin client libraries

We want one that supports SPV and BIP32.

- [picocoin/libccoin](https://github.com/jgarzik/picocoin)

Could work. I'm not sure how to work with its API.

- [bitc](https://github.com/bit-c/bitc)

Code is clean, but the API is not separate from the ncurses UI, so it would have to
be made into a library first, or mashed into this project by copy-and-paste.It
also uses its own event loop, which would have to be made to work with GTK's
event loop.

- [libbitcoin](https://github.com/libbitcoin/libbitcoin/)

Doesn't support SPV.
