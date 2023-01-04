# startOmatic
Sailing Boat Race Start Timer for Ardino

# This was built around the LED MATRIX displays from Freetronics (and knockoffs)
https://www.freetronics.com.au/products/dot-matrix-display-32x16-red

The main display is daylight readable in tropical sunlight used by Darwin Sailing Club

Uses various technologies
  - Lora
  - GPS
  - SPI
  - I2C

There are 2 devices:
  - A Large big 2048 LED display.
  - A Remote control box.

There's some documents here on how to build the hardware are here:
http://www.oceanhippie.net/content/guides/startOmatic

Know issues with current Files
- Main Display has no buttons local or otherwise inplimented - only works via remote
- Main Display can't handle GPS for sternchaser mode.
- Both have redundent or legacy code related to a mate's project wihch this maybye compatible with references to 'Signon' and 'Finish Times' are probably redudent
- Layout of both LCD's could be improved.
