# mem-mapped

Windows specific memory mapped file implementation. The main goal is speed in single threaded usage.

### Strong sides
 - Write to disk will happen only when data is actually changed
 - If provided with enough memory, the whole file will be mapped in memory
 - Memory page managment is FIFO for the pages usage
