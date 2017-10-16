Descriptors in XMOS USB Audio
=============================

ADAT Alternate Settings
-----------------------

6 interfaces

endpoint can only be shared between alternate settings of the same interface (not between interfaces)

change alternate settings via SET_INTERFACE request

an alternate setting has its' own interface and endpoint descriptors

+-----------+------+-------+----------------+--------------------------------------------------+
| interface | type | alt.  | endps.         | note                                             |
+-----------+------+-------+----------------+--------------------------------------------------+
| 0         | AC           | irq_in         |                                                  |
+-----------+------+-------+----------------+--------------------------------------------------+
| 1         | AS   | 0     |                | null interface                                   |
|           |      +-------+----------------+--------------------------------------------------+
|           |      | 1     | iso_out iso_in | 18 channels, host -> device, iso_in is feedback  |
|           |      +-------+----------------+--------------------------------------------------+
|           |      | 2     | shared         | 18 channels                                      |
+-----------+------+-------+----------------+--------------------------------------------------+
| 2         | AS   | 0     |                | null interface                                   |
|           |      +-------+----------------+--------------------------------------------------+
|           |      | 1     | iso_in         | 18 channels, device -> host                      |
|           |      +-------+----------------+--------------------------------------------------+
|           |      | 2     | shared         | 14 channels                                      |
+-----------+------+-------+----------------+--------------------------------------------------+
| 3         | AC           |                | MIDI                                             |
+-----------+------+-------+----------------+--------------------------------------------------+
| 4         | MS           | bul_out bul_in |                                                  |
+-----------+------+-------+----------------+--------------------------------------------------+
| 5         | DFU          |                |                                                  |
+-----------+------+-------+----------------+--------------------------------------------------+

Interface 1: host -> device

Interface 2: device -> host

Structure of defines::

                alt 0
                alt 1
                  18 ch
                #ifdef ADAT
                alt 2
                  14 ch
                #endif


Inconsistencies
---------------

- is MIDI AC interface is required? spec says 1 AC interface per function
- interface association descriptor specifies 3 interfaces, but there are 6
