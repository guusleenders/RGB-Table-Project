RGB Table Project
=================
A quite large 8x8 RGB LED matrix built into a table.
For more information, go to [guusleenders.me/rgbtable](http://guusleenders.me/rgbtable)

End Result
----------
Introduction video: [youtu.be/tsbG_MXWmow](http://youtu.be/tsbG_MXWmow)
More pictures: [flic.kr/s/aHsjL1AEgT](http://flic.kr/s/aHsjL1AEgT)

Construction
------------
The table is a hacked IKEA LACK Table, it's length and width are both 55cm. Including the semi-transparent box, the table is 56cm heigh.

Electronics
-----------
The electronics used in this table are really quite simple. I used an Arduino board as micro controllers that control the LED Drivers who in their turn control the LEDs itself. Another Arduino provides the interface for controlling the matrix.

Programming
-----------
Programming is entirely done in the Arduino programming environment. I've written my own library for displaying data on the table (see below). Communication is done in the sketch itself. The controller is programmed with a sketch (no custom made library).



If you have any questions, just want to say hi or anything else, you can always contact me:
- Twitter: @guusleendersme
- Mail: hello@guusleenders.me
