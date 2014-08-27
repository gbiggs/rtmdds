Building
--------

The examples can be built by making a directory (anywhere on your system
where you have write permissions will do), changing to that directory
and executing CMake with the examples' source directory as an argument.
For example, if you have installed rtmdds into /usr/local, you could do
the following:

$ cd ~
$ mkdir rtmdds_examples
$ cd rtmdds_examples
$ cmake /usr/local/share/rtmdds/examples
$ make

Running
-------

Launch the two stand-alone components:

$ ./rtmdds_pubcomp_standalone
$ ./rtmdds_subcomp_standalone

Next, connect each component's port to the topic. This can be done using
the rtcon command from rtshell:

$ rtcon pubcomp0.rtc:PublisherPort subcomp0.rtc:SubscriberPort

This will create a new DDS topic specifically for these two ports to
communicate over.

Alternatively, if you want to connect the ports to a specific topic (for
example, a topic that other ports or other DDS programs are already
using), you can specify it as a property of the connection. For example:

$ rtcon pubcomp0.rtc:PublisherPort subcomp0.rtc:SubscriberPort
    -p ddsport.topic=the_topic

Or, if you only want to connect a single component:

$ rtcon subcomp0.rtc:SubscriberPort -p ddsport.topic=the_topic

Configuration files
-------------------

The included sample configuration files demonstrate the parameters that
can be set on DDS ports. Parameters can be set on a specific port by
specifying that port's name, or on all publisher or subscriber ports in
general.

