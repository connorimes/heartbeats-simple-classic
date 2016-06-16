# Simple Heartbeats - Classic

Provides a classic Heartbeats style interface to [heartbeats-simple](https://github.com/libheartbeats/heartbeats-simple).

## Dependencies

The `pkg-config` utility is required during build to locate some of these dependencies.

* OpenMP (tested with [libgomp](https://gcc.gnu.org/projects/gomp/) 5.3.1)
* [heartbeats-simple >= 0.3.0](https://github.com/libheartbeats/heartbeats-simple)
* [energymon-default >= 0.2.0](https://github.com/energymon/energymon)

## Building

This project uses CMake.

To build, run:

``` sh
mkdir _build
cd _build
cmake ..
make
```

## Installing

To install, run with proper privileges:

``` sh
make install
```

On Linux, installation typically places libraries in `/usr/local/lib` and
header files in `/usr/local/include`.

## Uninstalling

Install must be run before uninstalling in order to have a manifest.

To uninstall, run with proper
privileges:

``` sh
make uninstall
```

## Usage

The following code snippet is an example of standard heartbeat usage.

``` C
  const uint64_t WINDOW_SIZE = 20;
  const char* LOG_FILE = "heartbeat.log";
  const uint64_t ITERATIONS = 1000;
  const uint64_t WORK_PER_ITERATION = 10;
  hbsc_ctx hb;
  uint64_t i, j;
  // initialize
  hbsc_init(&hb, WINDOW_SIZE, LOG_FILE);
  // should first issue a starter heartbeat to initialize start values (tag and work values are ignored)
  hbsc(&hb, 0, 0);
  for (i = 0; i < ITERATIONS / WORK_PER_ITERATION; i++) {
    // do some amount of application work
    for (j = 0; j < WORK_PER_ITERATION; j++) {
      do_work();
    }
    // issue a heartbeat
    hbsc(&hb, i, WORK_PER_ITERATION);
  }
  // cleanup
  hbsc_finish(&hb);
```

Note the starter heartbeat before work actually begins - if it is not issued, the first record will be lost.
This is used rather than setting the start values in the init function so that delays between heartbeat initialization and starting work do not impact the timing results.
