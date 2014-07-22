/**
@page GettingStarted Getting Started - Installation and Configuration

This is a basic guide to installing, testing, configuring and using the Assimilation Monitoring
system.
Please let the <a href="http://lists.community.tummy.com/cgi-bin/mailman/listinfo/assimilation">mailing list</a>
know if you try this out, if you run into problems, or if it works for you.

You can subscribe to the mailing list <a href="http://lists.community.tummy.com/cgi-bin/mailman/listinfo/assimilation">here</a>
and send emails to <a href="mailto:assimilation@lists.community.tummy.com">assimilation@lists.community.tummy.com</a>.
Please let us know how you're doing on this!

A normal installation consists of one instance of a CMA (collective management authority)
and <i>n+1</i> nanoprobes.  Only one machine runs the CMA software, but every machine being
monitored (including the CMA itself) runs a copy of the nanoprobe daemon.

There is a package for the nanoprobe daemon, called <i>assimilation-nanoprobe</i>, and a separate 
package for the CMA called (unsurprisingly) <i>assimilation-cma</i>.

The CMA also requires the <a href="http://neo4j.org">Neo4j</a> graph database to store
all the configuation and status information for the collection of machines
(aka the <i>collective</i>).  The CMA also requires the <a href="http://py2neo.org">py2neo</a>
package to talk to <i>neo4j</i>.

This document is a more detailed version of the information provided in the project
<a href="http://linux-ha.org/source-doc/assimilation/html/_r_e_a_d_m_e.html">README</a> file.

@section BuildPrereqs Build, Test and Documentation Prerequisites

The following packages are needed for building the packages, testing them, or creating documentation.

<b>Build-only packages</b>
- <a href="cmake.org">cmake</a> and cpack
- <a href="http://www.gnu.org/software/gcc/">gcc</a>,
  <a href="http://www.gnu.org/software/make/">make</a> and friends
- <a href="http://www.freedesktop.org/wiki/Software/pkg-config">pkg-config</a>
- <a href="http://code.google.com/p/ctypesgen/">ctypesgen</a> (for CMA code)

<b>Test-only packages</b>
- <a href="http://valgrind.org/">valgrind</a> (for testing 'C' code)
- <a href="https://github.com/Yelp/Testify">Testify</a> - Python testing framework (for CMA code)
- <a href="http://pylint.org/">Pylint</a> - Python code analysis (for CMA code)

<b>Nanoprobe (and CMA library) packages</b>
- <a href="http://developer.gnome.org/glib">glib2-dev</a> (aka libglib2.0-dev)
- <a href="http://www.tcpdump.org">libpcap-dev</a> (or winpcap for Windows)
- <a href="https://github.com/ClusterLabs/resource-agents">OCF resource agents</a> (available as 'resource-agents' package on Ubuntu)

<b>CMA-only packages</b>
- <a href="http://www.neo4j.org/install">Neo4j</a> graph database.  Note that Neo4j needs Java.  They prefer Oracle's Java.
- <a href="http://www.python.org/">Python 2.7</a> (approximately) interpreter for the Python language
- <a href="http://py2neo.org/">py2neo</a> Python bindings for Neo4j - version <b>1.6.1</b> or later
- <a href="https://pypi.python.org/pypi/netaddr">python-netaddr</a> Python network address package
- <a href="https://pypi.python.org/pypi/getent">getent</a> Python library for reading UNIX password and group files
- <a href="http://flask.pocoo.org/">flask</a> Python web microframework

<b>Documentation packages</b>
- <a href="http://doxygen.org">Doxygen</a> - for producing this documenation
- dot (part of <a href="http://www.graphviz.org/">graphviz</a>) for creating graphs for the source documentation - and handy for printing graphs from Neo4j.

All of the Nanoprobe and documentation packages are readily available on most modern operating systems.

Depending on your environment, you may not have OS-level packages for all of the python
pieces (testify, ctypesgen, py2neo, flask).
If you don't have OS packages for those pieces, then you can use
<a href="https://pypi.python.org/pypi/pip">pip</a> to easily install them.

@subsection InstallingNeo4j Installing Neo4j
There are no packages available for Neo4j, so you have to install it from their tar ball
following their
<a href="http://docs.neo4j.org/chunked/stable/server-installation.html">directions</a>.
There is also a short
<a href="http://www.neo4j.org/install#installvideo">video installation guide</a> as well.
The Assimilation code has been tested most recently with the <i>Community 1.9M04</i> version.
Fortunately, they do provide an init script for it, so it can easily be started as a service,
called <i>neo4j-service</i>.
It must be started before the CMA starts.

@section GettingOurSource Getting a copy of the Assimilation source code
There are three ways you can get a copy of the Assimilation Project source - as a tar ball
or in a <a href="http://mercurial.selenic.com/">Mercurial</a> source code repository,
or if you run a Red-Hat based release, you can get source RPMs suitable for recent red hat releases.

You can obtain prebuilt packages of the project (currently only for Ubuntu),  using a web browser <a href="www.gnu.org/software/wget/">wget</a>.

 - Bleeding edge builds - https://www.dropbox.com/sh/h32lz3mtb8wwgmp/XM-oTLWWdm
 - Officially released versions - https://www.dropbox.com/sh/pd9i31q1hj39l3i/9znelMfmtT

You can obtain a <i>tar ball</i> of the source using a web browser, or <a href="www.gnu.org/software/wget/">wget</a>.

 - Bleeding edge - http://hg.linux-ha.org/%7Cexperimental/assimilation/archive/tip.tar.gz
 - Latest stable version - http://hg.linux-ha.org/%7Cexperimental/assimilation/archive/v0.1.0.tar.gz


If you'd rather have an active source code repository to use to watch the development
or contribute to it, then follow these steps:
- Install Mercurial
- cd <i><some-suitable-source-directory</i>
- <tt><b>hg clone 'http://hg.linux-ha.org/%7Cexperimental/assimilation/'</b></tt>

If you are running a RedHat-based release and you are happy to deal with source RPMs, then feel
free to use Jamie Nguyen's source RPMs or spec files which you can find here:
- http://jamielinux.fedorapeople.org/assimilation/


This completes all the preparation necessary to begin building the system.

@section BuildingTheCode Building The Code
- create a <i>new-binary-directory</i> to hold the completed binaries - separate from the source tree above
- cd <i>new-binary-directory</i>
- cmake <i>pathname-of-source-directory</i>
- cpack

This should produce two packages - one named <i>assimilation-nanoprobe</i> and one named <i>assimilation-cma</i>.

If you run into difficulties, it is likely the result of missing or incorrect dependencies.
It is also possible that you're compiling for an older system - but this is likely solvable.
Please report any problems you encounter to the mailing list.
The packages seem to be usable when you make them for Ubuntu.  Other OSes may or may make good packages yet.

@section MakingTheDocs Making The Documentation (this web site)
To make the documentation for the project, issue this command
- <tt>make doc</tt>

This will create all the documentation on the <i>assimmon.org</i> web site.

@section InstallingTheCode Installing the Code
Every machine you wish to discover and monitor, including the CMA, must have the nanoprobe
code installed on it.  The CMA code makes extensive use of the libraries created
for the nanoprobes.

@subsection InstallingTheNanoprobeCode Installing the Nanoprobe code
This is how to install the nanoprobe code on Debian-based systems:
- <tt>sudo dpkg-install assimilation-nanoprobe-<i>version-architecture</i>.deb</tt>

This is how to install the nanoprobe code on RPM-based systems
- <tt>sudo rpm --install assimilation-nanoprobe-<i>version-architecture</i>.rpm</tt>

@subsection InstallingTheCMA Installing the CMA Code
This is how to install the CMA code on Debian-based systems:
- <tt>sudo dpkg-install assimilation-cma-<i>version-architecture</i>.deb</tt>

This is how to install the CMA code on RPM-based systems
- <tt>sudo rpm --install assimilation-cma-<i>version-architecture</i>.rpm</tt>

If you are unable to build an RPM or DEB package, you can use <tt>sudo make install</tt>.
This installs both the nanoprobe and CMA code.
However, if you do, you will need to also issue this command:
<pre>
    ldconfig /usr/lib/assimilation
</pre>

@subsection TestifyTests testify tests
There are a large number of tests performed on the Python code
including the CMA code with database.
The project runs these tests before updating the master source
control instance on <i>hg.linux-ha.org</i>.
These regression tests also significantly exercises the C code underlying the
python code, and the interfaces between the two bodies of code.
These tests bind to port 1984, so some of them will fail if port 1984 is not available.

To run these tests, execute these steps:
- <tt>cd <i><source-code-directory></i>/cma</tt>
- <tt>testify tests</tt>

The final line should look something like this:
<pre>PASSED.  74 tests / 22 cases: 74 passed, 0 failed.  (Total test time 172.75s)</pre>


@subsection ValgrindTest testcode/grind.sh test
This code is a pure-C test which exercises the nanoprobe code with
a simulated CMA.  It is run under valgrind to look for memory leaks
outside our object system (those are noted automatically).
There is a variety of hard-coded IP addresses used in these tests.
This can be ignored for the time being.
However, the test binds to port 1984, so it will fail if port 1984 is not available.

This test is now run automatically by Testify - so see the section above for how to run it.

Various things in the glib2 library do not free all their memory at exit, so
it is possible (likely?) that you will see things not being freed that are harmless.
Nevertheless, please report them to the mailing list.

Normal output looks something like this:
<pre>
** Message: Our OS supports dual ipv4/v6 sockets. Hurray!
** Message: Joining multicast address.
** Message: multicast join succeeded.
** Message: CMA received startup message from nanoprobe at address [::1]:1984/1984.
** Message: PARSED JSON: {"source":"netconfig","discovertype":"netconfig","description":"IP Network Configuration","host":"servidor","data":{"virbr0":{"ipaddrs":{"192.168.122.1/24":{"brd":"192.168.122.255","scope":"global","name":"virbr0"}},"mtu":1500,"address":"fe:54:00:9e:57:e7","carrier":true,"operstate":"up"},"vnet0":{"ipaddrs":{"fe80::fc54:ff:fe9e:57e7/64":{"scope":"link"}},"mtu":1500,"address":"fe:54:00:9e:57:e7","carrier":true,"duplex":"full","operstate":"unknown","speed":10},"eth0":{"default_gw":true,"duplex":"full","carrier":true,"speed":1000,"address":"00:1b:fc:1b:a8:73","ipaddrs":{"10.10.10.5/24":{"brd":"10.10.10.255","scope":"global","name":"eth0"},"fe80::21b:fcff:fe1b:a873/64":{"scope":"link"}},"operstate":"up","mtu":1500},"lo":{"ipaddrs":{"127.0.0.1/8":{"scope":"host","name":"lo"},"::1/128":{"scope":"host"}},"mtu":16436,"address":"00:00:00:00:00:00","carrier":true,"operstate":"unknown"}}}
** Message: 1 JSON strings parsed.  0 errors.
** Message: Connected to CMA.  Happiness :-D
** Message: CMA Received switch discovery data (type 31) over the 'wire'.
** (process:4565): WARNING **: Peer at address 10.10.10.4:1984 is dead (has timed out).
** Message: CMA Received dead host notification (type 26) over the 'wire'.
** Message: QUITTING NOW! (heartbeat count)
** (process:4565): WARNING **: _fsprotocol_send1.855: Attempt to send FrameSet while link shutting down - FrameSet ignored.
** (process:4565): WARNING **: _fsproto_fsa.222: Got a 5 input for [::1]:1984/0 while in state 0
** (process:4565): WARNING **: _fsproto_fsa.225: Frameset given was: FrameSet(fstype=17, [[{SignFrame object at 0x0x6121da0}], [SeqnoFrame(type=4, (272464917,0,7))], [{Frame object at 0x0x6121f40}]])
** Message: Count of 'other' pkts received:     2
** Message: No objects left alive.  Awesome!
</pre>

The <i>CMA Received switch discovery data</i> message will not occur unless the OS
you're running it on has a NIC directly connected to an LLDP-equipped switch
(CDP is not yet fully supported).

You may also get a variety of debug messages, depending on the version of glib2 you have
installed.  This code has a lot of debug enabled, but later versions of glib2 suppress
debug messages unless the the environment variable G_MESSAGES_DEBUG is set to <b>all</b>.

@subsection PingerTest testcode/pinger
The pinger program exercises the reliable UDP retransmission code in the
project.  It is hard-wired to use port 19840.  Hope that works for you.
It sends a number of packets with 5% simulated packet reception loss
and 5% simulated packet transmission loss.  This is 9.75% overall packet loss rate.

This test is now automatically run as part of the testify tests, so see the testify section above.

Because it has a lot of debug enabled, debug might or might not come out by default
depending on what version of glibc2 you have installed.
Nevertheless, at the end you should be able to see these messages:
<pre>
Received a PING packet (seq 7) from [::1]:19840 ========================
Sending a PONG(2)/PING set to [::1]:19840
Received a PONG packet from [::1]:19840
Received a PONG packet from [::1]:19840
Received a PING packet (seq 8) from [::1]:19840 ========================
Sending a PONG(2)/PING set to [::1]:19840
Received a PONG packet from [::1]:19840
** Message: _netio_recvapacket: Threw away 66 byte input packet
** Message: _netio_recvapacket: Threw away 74 byte input packet
Received a PONG packet from [::1]:19840
Received a PING packet (seq 9) from [::1]:19840 ========================
Sending a PONG(2)/PING set to [::1]:19840
Received a PONG packet from [::1]:19840
Received a PONG packet from [::1]:19840
** Message: Shutting down on ping count.
Received a PING packet (seq 10) from [::1]:19840 ========================
Sending a PONG(2)/PING set to [::1]:19840
Received a PONG packet from [::1]:19840
Received a PONG packet from [::1]:19840
ALL CONNECTIONS SHUT DOWN! calling g_main_quit()
** Message: No objects left alive.  Awesome!
</pre>

Because the packet loss is random, the various <i>Threw away...</i> messages
will likely be in different places.
But it <i>should</i> stop, and it should end with the <i>Awesome!</i> message.


@section ConfiguringTheServices Configuring the Services
There is currently no configuration needed for these systems
under most circumstances.  If your network does not support multicast,
then you will have to invoke the nanoprobes with an argument
specifying the address of the CMA.
By default communication takes place on UDP port 1984.
If port 1984 is not available to the nanoprobes, it will bind to an ephemeral port.

This happens every time when starting a nanoprobe on the CMA -
since the CMA has already bound to that port.

@section DealingWithFirewalls Dealing With Firewalls
Some systems (RHEL for example) come configured out of the box
with a default iptables configuration which will block our communication.
As you might guess, things don't work too well under those circumstances.

To write firewall (iptables) rules to allow our communication, it is necessary to understand how
the Assimilation code communicates.
All our communication uses the UDP protocol.
The CMA and all the nanoprobes <i>except the one running on the CMA machine</i> default to
using UDP port 1984.  However, both the nanoprobe and the CMA can't use
port 1984 at the same time, so if the nanoprobe can't use it's requested port,
it will use an ephemeral port.
As long as there is only one system using an ephemeral port, then all communication
will have either a source or a destination port of 1984.

For this (normal) case, the following firewall rules will allow our software to communicate.
<pre>
-A INPUT -m udp -p udp --dport 1984 -j ACCEPT
-A INPUT -m udp -p udp --sport 1984 -j ACCEPT
</pre>
For non-CMA machines, they should only need the first rule.
The CMA will need both rules added to its rule set if iptables would otherwise block the communication.

@section ActivatingTheServices Activating The Services
As of this writing, the packages we install do not activate the services, 
so you will need to activate them manually. Sorry :-(

@subsection StartingNeo4j Starting the Neo4j Database
- <tt>service neo4j-service start</tt>

@subsection StartingAssimilationCode Starting the Assimilation Code
Keep in mind that you need to install and start nanoprobes on every machine,
but you should only to start the <i>cma</i> service on one machine.

On Debian-based systems:
- <tt>/usr/sbin/update-rc.d nanoprobe defaults</tt>
- <tt>/usr/sbin/update-rc.d cma defaults</tt>
- <tt>service cma start</tt>
- <tt>service nanoprobe start</tt>

On SuSE systems
- <tt>insserv nanoprobe</tt>
- <tt>insserv cma</tt>
- <tt>service cma start</tt>
- <tt>service nanoprobe start</tt>

On RedHat systems
- <tt>chkconfig --add nanoprobe</tt>
- <tt>chkconfig --add cma</tt>
- <tt>service cma start</tt>
- <tt>service nanoprobe start</tt>

On LSB-compliant systems
- <tt>/usr/lib/lsb/install_initd nanoprobe</tt>
- <tt>/usr/lib/lsb/install_initd cma</tt>
- <tt>service cma start</tt>
- <tt>service nanoprobe start</tt>

If for some reason while playing around, you need to reinitialize the database, then next time
start the CMA with the --erasedb flag.

@section ReadingTheLogs Reading System Logs
The nanoprobe code and the CMA code operate as normal daemons.
That is, they put themselves in the background and everything worth knowing is put into
the system logs.

Below are a few interesting messages which you can expect to see,
along with explanations of what they mean.

@subsection CMAStartUpMessages CMA startup messages
<pre>
Mar  3 14:20:45 servidor cma INFO: Listening on: 0.0.0.0:1984
Mar  3 14:20:45 servidor cma INFO: Requesting return packets sent to: 10.10.10.5:1984
Mar  3 14:20:45 servidor cma INFO: Starting CMA version 0.1.0 - licensed under The GNU General Public License Version 3
</pre>

The CMA has started up, is listening to ANY port 1984, and is telling nanoprobes to send their
packets to address 10.10.10.5, port 1984.  Currently messages printed from the CMA by the C code have the process id
in them, and messages from the python code do not.

@subsection NanoprobeStartUpMessages Nanoprobe Startup Messages
<pre>
Mar  3 14:23:14 servidor nanoprobe[17660]: INFO: CMA address: 224.0.2.5:1984
Mar  3 14:23:14 servidor nanoprobe[17660]: INFO: Local address: [::]:45714
Mar  3 14:23:14 servidor nanoprobe[17660]: INFO: Starting version 0.1.0: licensed under The GNU General Public License Version 3
Mar  3 14:23:17 servidor cma INFO: Drone servidor registered from address [\::ffff:10.10.10.5]:45714 (10.10.10.5:45714)
Mar  3 14:23:17 servidor nanoprobe[17660]: NOTICE: Connected to CMA.  Happiness :-D
Mar  3 14:23:19 servidor cma INFO: Stored arpcache JSON data from servidor without processing.
Mar  3 14:23:20 servidor cma INFO: Stored cpu JSON data from servidor without processing.
Mar  3 14:23:21 servidor cma INFO: Stored OS JSON data from servidor without processing.
</pre>
This means that a nanoprobe has stared up, process id 17660, and will try and locate the CMA by sending a packet
to the (multicast) address 224.0.2.5, port 1984.
It is listening to packets sent to ANY address, port 45714.
The port used is 45714 instead of 1984 because this nanoprobe is on the same machine as the CMA,
which already bound to [::]:1984.  Nanoprobes on other machines will normally show 
<tt>Local address: [::]:1984</tt> instead.
The "Stored ... JSON data from ... without processing" messages mean that we received new (different)
information for this discovery module than we had in the database, and that we
just stored it.
These particular discovery items have no special actions taken when they arrive - they're just stored
in the database.

@subsection NanoprobeShutdownMessages Nanoprobe Shutdown Messages
The messages below were the result of a <tt>service nanoprobe stop</tt> command.
<pre>
Mar  3 14:30:55 servidor nanoprobe[18879]: NOTICE: nanoprobe: exiting on SIGTERM.
Mar  3 14:30:55 servidor cma INFO: System servidor at [\::ffff:10.10.10.5]:45714 reports it has been gracefully shut down.
Mar  3 14:30:55 servidor nanoprobe[18879]: INFO: Count of heartbeats:                       0
Mar  3 14:30:55 servidor nanoprobe[18879]: INFO: Count of deadtimes:                        0
Mar  3 14:30:55 servidor nanoprobe[18879]: INFO: Count of warntimes:                        0
Mar  3 14:30:55 servidor nanoprobe[18879]: INFO: Count of comealives:                       0
Mar  3 14:30:55 servidor nanoprobe[18879]: INFO: Count of martians:                         0
Mar  3 14:30:55 servidor nanoprobe[18879]: INFO: Count of LLDP/CDP pkts sent:               1
Mar  3 14:30:55 servidor nanoprobe[18879]: INFO: Count of LLDP/CDP pkts received:          27
Mar  3 14:30:55 servidor nanoprobe[18879]: INFO: Count of recvfrom calls:                  28
Mar  3 14:30:55 servidor nanoprobe[18879]: INFO: Count of pkts read:                       13
Mar  3 14:30:55 servidor nanoprobe[18879]: INFO: Count of framesets read:                  13
Mar  3 14:30:55 servidor nanoprobe[18879]: INFO: Count of sendto calls:                    14
Mar  3 14:30:55 servidor nanoprobe[18879]: INFO: Count of pkts written:                    14
Mar  3 14:30:55 servidor nanoprobe[18879]: INFO: Count of framesets written:                0
Mar  3 14:30:55 servidor nanoprobe[18879]: INFO: Count of reliable framesets sent:         10
Mar  3 14:30:55 servidor nanoprobe[18879]: INFO: Count of reliable framesets recvd:         2
Mar  3 14:30:55 servidor nanoprobe[18879]: INFO: Count of ACKs sent:                        3
Mar  3 14:30:55 servidor nanoprobe[18879]: INFO: Count of ACKs recvd:                      10
Mar  3 14:30:55 servidor nanoprobe[18879]: INFO: Count of 'other' pkts received:            0
Mar  3 14:30:55 servidor nanoprobe[18879]: INFO: No objects left alive.  Awesome!
</pre>

The nanoprobe announced it was exiting, the CMA acknowledged that the system was shutting down
gracefully, the nanoprobe then printed out various statistics, and finally ended
with the <i>Awesome!</i> message indicating no memory leaks were observed.

@subsection NanoprobeCrashMessages Nanoprobe Crash Messages
The messages below occurred when the system running a nanoprobe or the nanoprobe itself crashed.
In this case, the nanoprobe process running on paul was killed with SIGKILL (kill -9) which simulates a server crash.
<pre>
Mar 11 12:27:27 servidor nanoprobe[6416]: WARN: Peer at address [\::ffff:10.10.10.16]:1984 is dead (has timed out).
Mar 11 12:27:27 servidor cma WARNING: DispatchHBDEAD: received [HBDEAD] FrameSet from [[\::ffff:10.10.10.5]:44782]
Mar 11 12:27:27 servidor cma INFO: Node paul has been reported as dead by address [\::ffff:10.10.10.5]:44782. Reason: HBDEAD packet received
Mar 11 12:27:28 servidor cma WARNING: DispatchHBDEAD: received [HBDEAD] FrameSet from [[\::ffff:10.10.10.2]:1984]
Mar 11 12:27:28 servidor cma INFO: Node paul has been reported as dead by address [\::ffff:10.10.10.2]:1984. Reason: HBDEAD packet received
</pre>

Note that the dead node (paul) was reported as being dead from the two peers monitoring it.
Since one of paul's peers was the node running the CMA (servidor), the first message "<tt>Peer at address...is dead</tt>"
from the nanoprobe also appears in the logs on this machine.


@subsection CMACrashMessages CMA Crash Messages
If you should see the CMA misbehave, it will probably either disappear with a crash
(indicating a problem in the interfaces to the C code), or it will catch an
exception handling a message from a client.
The messages below are typical of what you would see should this unfortunate event occur:
<pre>
Mar  3 14:50:08 servidor cma CRITICAL: MessageDispatcher exception [Relationship direction must be an integer value] occurred while handling [HBDEAD] FrameSet from [\::ffff:10.10.10.2]:1984
Mar  3 14:50:08 servidor cma INFO: FrameSet Contents follows (1 lines):
Mar  3 14:50:08 servidor cma INFO: HBDEAD:{SIG: {SignFrame object at 0x0x1e56680}, pySeqNo(REQID: (0, 1)), IPPORT: IpPortFrame(13, [\::ffff:10.10.10.16]:1984), END: {Frame object at 0x0x1edd890}}
Mar  3 14:50:08 servidor cma INFO: ======== Begin HBDEAD Message Relationship direction must be an integer value Exception Traceback ========
Mar  3 14:50:08 servidor cma INFO: messagedispatcher.py.51:dispatch: self.dispatchtable[fstype].dispatch(origaddr, frameset)
Mar  3 14:50:08 servidor cma INFO: dispatchtarget.py.61:dispatch: deaddrone.death_report('dead', 'HBDEAD packet received', origaddr, frameset)
Mar  3 14:50:08 servidor cma INFO: droneinfo.py.269:death_report: hbring.HbRing.ringnames[ringname].leave(self)
Mar  3 14:50:08 servidor cma INFO: hbring.py.178:leave: relationships = drone.node.get_relationships('all', self.ournexttype)
Mar  3 14:50:08 servidor cma INFO: neo4j.py.1190:get_relationships: uri = self._typed_relationships_uri(direction, types)
Mar  3 14:50:08 servidor cma INFO: neo4j.py.1161:_typed_relationships_uri: raise ValueError("Relationship direction must be an integer value")
Mar  3 14:50:08 servidor cma INFO: ======== End HBDEAD Message Relationship direction must be an integer value Exception Traceback ========
</pre>

This particular set of messages were caused by a mismatch between the CMA code and the version of the <i>py2neo</i> code.
Note the <b>CRITICAL: MessageDispatcher exception</b> message that started it all off.
The next line contains a dump of the message that triggered the falure, followed by 
a stack trace formatted to be passably readable in syslog.


@section EnablingDebugging Enabling Debugging
Both the CMA and the nanoprobe process take a <b>-d</b> flag to increment the debug level
by one.  Currently debug values between 1 and 5 produce increasing levels of detail.
In addition, while the nanoprobe code is running, its debug level can be modified
at run time by sending it signals.  If you send it a <b>SIGUSR1</b> signal
the overall debug level will be raised by one.  If you send it a <b>SIGUSR2</b> signal,
the overall debug level will be lowered by one - unless it is already at zero, in which
case the <b>SIGUSR2</b> will be ignored.

@section ExaminingNeo4j Examining the Neo4j database
Neo4j comes with an Administrative  web server for examining various aspects of the database.
It can be reached at <a href="http://localhost:7474/webadmin/">http://localhost:7474/webadmin/</a>.

The tabs you should find there include:

 - <b>Overview Dashboard </b> - provides an overview of the number of nodes, relationships and properties over time
 - <b>Explore and edit</b> - Visual Data browser - visually display the result of a <a href="http://www.neo4j.org/learn/cypher">Cypher</a> query.  It's also interactive, to allow to arrange nodes on the screen.
 - <b>Power tool Console</b>A low-level shell-like language for exploring the database.
   Can also be invoked as <tt>neo4j-shell</tt> from the command line.
 - <b>Add and remove indexes</b> - you probably don't want to do this
 - <b>Server Info</b> - information about how this Neo4j server is configured
@section CoolCypherQueries A few Cool Cypher queries
Below you'll find a number of useful and interesting Cypher queries which you
can issue from the Neo4j Administrative web server mentioned above, or you can
embed them in your programs.
The list below is far from exhaustive, but should be sufficient to give a few ideas
of the types of things that can be done.

In order to fully appreciate the kinds of queries that one might perform, it is
necessary to understand Assimilation Project's Neo4j schema.
This schema was outlined in a number of blog postings - relating to the overall
<a href="http://techthoughts.typepad.com/managing_computers/2012/08/an-assimilation-type-schema-in-neo4j.html">nodetype schema</a>,
<a href="http://techthoughts.typepad.com/managing_computers/2012/07/neo4j-server-schema-for-the-assimilation-project.html">Servers and IP addresses</a>,
<a href="http://techthoughts.typepad.com/managing_computers/2012/07/assimilation-ring-neo4j-schema.html">rings</a>,
<a href="http://techthoughts.typepad.com/managing_computers/2012/07/discovering-switches-its-amazing-what-you-can-learn-just-by-listening.html">switches and switch connections</a>,
and lastly
<a href="http://techthoughts.typepad.com/managing_computers/2012/07/clients-servers-and-dependencies-oh-my.html">clients, servers and dependencies"</a>.



@subsection GetTheServerList Retrieve The List of Servers
<pre>
START root=node(0)
MATCH drone-[:IS_A]->type-[:IS_A]->root
WHERE type.name = "Drone"
RETURN drone
</pre>
This will bring up a table with the nodes in the graph for servers (Drones) in the database.
If you click on any of the items in the graph, it will show all the basic properties for a server.
This list should include these items:
 - <tt>port</tt>: - the port the nanoprobe is listening on
 - <tt>nodetype</tt>: "Drone"
 - <tt>status</tt>: - "up" or "down"
 - <tt>reason</tt>: - the reason for the last status update
 - <tt>name</tt>: hostname
 - <tt>iso8601</tt>: time of last status update in ISO 8601 format - <i>will probably go away in the future</i>
 - <tt>statustime</tt>: statustime - time of last status update - millseconds since 00:00 Jan 1, 1970 EST (the UNIX epoch)
 - <tt>JSON_arpcache</tt>: JSON from ARP cache discovery
 - <tt>JSON_cpu</tt>: JSON from cpu discovery
 - <tt>JSON_netconfig</tt>: JSON from network configuration discovery
 - <tt>JSON_OS</tt>: JSON from OS discovery
 - <tt>JSON_tcpclients</tt>: JSON from the tcpclients discovery
 - <tt>JSON_tcplisteners</tt>: JSON from the tcplisteners discovery
 - <tt>JSON_\#LinkDiscovery</tt>: JSON from link discovery (if you have an LLDP-equipped switch)

If you just want the list of host names, you can use this very similar query:
<pre>
START typeroot=node(0)
MATCH drone-[:IS_A]->nodetype-[:IS_A]->typeroot
WHERE nodetype.name = "Drone"
RETURN drone.name
</pre>

@subsection GetDownServers Retrieve The List of Down Servers
The query below returns the set of servers which are currently marked down - regardless of the reason
they're down.
<pre>
START typeroot=node(0)
MATCH drone-[:IS_A]->nodetype-[:IS_A]->typeroot
WHERE nodetype.name = "Drone" and drone.status = "dead"
RETURN drone
</pre>

@subsection GetShutDownServers Retrieve The List of Gracefully Shut Down Servers
The query below returns the set of servers which are currently down and were shut down gracefully.
<pre>
START typeroot=node(0)
MATCH drone-[:IS_A]->nodetype-[:IS_A]->typeroot
WHERE nodetype.name = "Drone" and drone.status = "dead" and drone.reason = "HBSHUTDOWN"
RETURN drone
</pre>

@subsection GetCrashedServers Retrieve The List of Crashed Servers
The query below returns the set of servers which are down but were <b>not</b> shut down gracefully
(i.e., they crashed).
<pre>
START typeroot=node(0)
MATCH drone-[:IS_A]->nodetype-[:IS_A]->typeroot
WHERE nodetype.name = "Drone" and drone.status = "dead" and drone.reason <> "HBSHUTDOWN"
RETURN drone
</pre>

@subsection GetCrashedServersWithTimes Retrieve The List of Crashed Servers and When They Crahsed
The query below returns the set of servers which are down but were <b>not</b> shut down gracefully
(i.e., they crashed) - with the systems that have been down the longest first.
<pre>
START typeroot=node(0)
MATCH drone-[:IS_A]->nodetype-[:IS_A]->typeroot
WHERE nodetype.name = "Drone" and drone.status = "dead" and drone.reason <> "HBSHUTDOWN"
RETURN drone, drone.iso8601
ORDER BY drone.iso8601
</pre>

@subsection GetNICConnections Return which server NICs are connected to which switch NICs
The query below will return which switch ports are connected to which server ports, along with the
SystemName of the switch, and the description of the switch port.
As of the current release, this query will only produce results if you have LLDP data available to your servers.
<pre>
START typeroot=node(0)
MATCH switch<-[:nicowner]-switchnic-[:wiredto]-dronenic-[:nicowner]->drone-[:IS_A]->nodetype-[:IS_A]->typeroot
WHERE nodetype.name = "Drone"
RETURN  drone.name, dronenic.nicname, switch.SystemName, switchnic.nicname, switchnic.PortDescription
</pre>
This should produce output which looks something like this:
<pre>
<b>drone.name dronenic.nicname switch.SystemName      switchnic.nicname switchnic.PortDescription</b>
servidor   eth0             GS724T_10_10_10_250    g6                Alan's office - North wall, grey jack
</pre>

@subsection GetRingMembers Return which servers are members of a given ring
The query below will return all the systems which are in the given ring (in this case "The_One_Ring").
For the current state of only one ring, this is another way to get the list of all up servers.
<pre>
START Ring=node:Ring(Ring="The_One_Ring")
MATCH Ring<-[RingMember_The_One_Ring]-Drone
RETURN Drone
</pre>

@subsection GetRingMembers Return servers on a ring with a node, in the order they appear on the ring
Thie query below will follow the <i>RingNext</i> links from the given node around the ring
until they return to the initial node.  It's kind of a funky little query...
<pre>
START Drone=node:Drone(Drone="drone000001")
MATCH Drone-[:RingNext_The_One_Ring*]->NextDrone
RETURN NextDrone.name, NextDrone
</pre>
The results should loke something like this:
<pre>
"drone000002"	[Node 31258]
"drone000003"	[Node 31261]
"drone000004"	[Node 31264]
"drone000005"	[Node 31267]
"drone000001"	[Node 31255]
</pre>
Note that this query returns the initial node <i>last</i>.


@subsection EvenMoreQueries Even More Cool Cypher Queries
These queries don't begin to scratch the surface of what you can do with the Assimilation
Monitoring project and Cypher queries into the Neo4j database.
So, now it's up to you!

Go forth, create even more Cool Cypher queries, and share them with everyone on the Assimilation
<a href="http://lists.community.tummy.com/cgi-bin/mailman/listinfo/assimilation">mailing list</a>.

The CMA code has a collection of canned queries.  You can read
these queries along with some metadata about them by looking at
the source files you find 
<a href="http://hg.linux-ha.org/assimilation/file/tip/queries">here</a>.

@section UnInstalling Un-installing
If you wish to uninstall the software, and you installed it as packages, please use the mechanism
that comes with the packaging for your operating system.

If you installed it with <tt>sudo make install</tt>, then there should be a file named
<tt>install_manifest.txt</tt> in the top directory of your build directory that lists all the
files that were installed.  Removing the files listed in that file should remove all the
installed files.

@section GettingStartedConclusion Conclusion
If you have executed all these steps, and everything has worked, then congratulations, everything is working!
Please let the <a href="http://lists.community.tummy.com/cgi-bin/mailman/listinfo/assimilation">mailing list</a> know!

If it didn't work for you, it's <i>even more</i> important to let the mailing list know.

*/
