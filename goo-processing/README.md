# Things I learned from trying to process dictionary entries

- Nice concurrency support in C++
- Can't just push a zillion futures into a queue, threads are still
  expensive to create
- A ThreadPool can be implemented nicely with C++17
- Use `iostat` and `htop` to figure out if your processes are in
  uninterruptible sleep and if there are high values for `%iowait`
- Using too many threads in thread pool will cause number of sleeping
  processes and `%iowait` to shoot up as they will start waiting and
  be preempted by other threads making the whole thing slower
- I got it to do the 12GB in about 30 seconds, would be fun to get it
  to go faster as well
