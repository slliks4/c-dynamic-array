# Things learnt so far

- sizeof != alignment

- raw bytes don't retain type information

- pointer + size is enough to copy arbitrary object representations

- alignment matters if raw storage will be cast back to typed pointers

- padding is part of memory layout

- logical index doesn't have to equal physical position

- fixed-size metadata gives O(1) lookup into variable-size storage

- moving packed data creates metadata consequences

- not moving packed data creates fragmentation
