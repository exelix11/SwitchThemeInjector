# hactool
Original repo: https://github.com/SciresM/hactool

Changes:
- Makefile to compile as a switch lib
- Use spl services to decrypt the key area of an nca
- Extract exefs files to a buffer

Portlibs mbedtls won't work for building as it's compiled without cmac support.