OBJECT :

Based on existing projects (LOIC, Loiq, Slowloris), design a lightweight and multi-platform solution, in C language.



CONTEXT :

LOIC (Low Orbit Ion Cannon) is an open source network stress tool.


Loic already exists in several versions :

	- Loic "standard", developed by Prateox (https://sourceforge.net/projects/loic/), then improved by NewEraCracker (http://github.com/NewEraCracker/LOIC/downloads)
		Developped in C#, it's the "source" software, so the most commonly used. The main problem is its portability (designed for Windows, its integration into Linux is not native)
	
	- Loiq (http://sourceforge.net/projects/loiq/) has the same functionality as LOIC (first), but is designed for UNIX platforms (developped in C++/Qt)

	- PyLoic (https://github.com/SlamDunkn/LOIC_Python/) is a variation of the software, developped in Python.
	
	...

There are many derivatives of Loic, each with its advantages, and its prerequisites.
	

	
Many improvements have been requested on the original software, but many are impossible. In addition, it is difficult to use LOIC Linux, making it necessary to test many versions (pyLoic, Loiq, etc).

The main goal is, initially, to build an equivalent LOIC in C, which has the same functionality, which works on both Linux and Windows.



The advantage of the C language is that is a "low level" programming language, so it can interact closely with the machine, which can multiply the possibilities
	

STEP 1 :
--> Make a clone of LOIC, in C langage, with the sames syntax and features. This program should work both on Linux and on Windows.


STEP 2 :
--> Enjoy the benefit of the C language to enhance certain features, by proposing new types of attack / test. (use Winpcap for Raw Sockets ? It Could Be possible !)
	
STEP 1/2 (parallel) :
--> Make a hidden mode (for Windows) and a daemon version (for Linux). Make a GUI (for Linux AND Windows, so maybe Gtk)



(Code and "README" under construction)




USAGE :

cLoic(.exe) method [ -m method ] [ -nt number_of_threads ] [ -s speed_in_percent ] [ -t target ] [ -p port ] [ -to seconds ] [ -w ] [ -r ] [ -gz ] [ -get ] [ -hs subsite ] [ -h irc.hive.server [ port #channel ] ] target

-m -a --method --attack : Method/Protocol used. Can be UDP, TCP, HTTP, or SLOWLORIS. The "SYN" or "SUDP" options are also available in SuperLOIC version (libpcap/WinpCap required)
(default : TCP)

-nt --threads : Number of parallel threads 
(default : 10)

-s --speed : Canon speed, in percent. 5 (%) to 100 (%). Use 200 (or > 100 value) for full speed mode.
(default : 90)

-t --target : Target host. Can be an IP adress, or a host name.

-p --port : Target port.
(default : 80)

-to --timeout : Time (in seconds) while the program will run.
(default : 9999)

-w --wait-for-response : Wait after each packet sent.
(no value)

-r --random : Append random string to HTTP subsite or UDP requests.
(no value)

-h --hive --hivemind : Connect LOIC to an IRC server, for remote control.
(ex : -h irc.yourircserver.la:6667 loic)

-get --slowloris-get (only in SlowLoris mode) : Use "get" method instead of "post", in Slowloris mode.
(no value)

-gz --gzip (in HTTP modes) : Accept "gzip" in HTTP requests.
(no value)

-hs --http-subsite (in HTTP modes) : Target a specified page in the Website.
(default : "/")


(following parameters are only availables in SuperLOIC version)

-i --interface (SYN and SUDP modes) : Specify the interface used to send packets.

-A --ip-spoof (SYN and SUDP modes) : Specify the source IP address sent to the target (can't work throught a NAT)


example : <<   cLoic(.exe) -m HTTP -to 10 -hs "/page.php" -t mywebsite.com -nt 5 -w    >>
( or <<    cLoic(.exe) HTTP -to 10 -w -hs "/page.php" -nt 5 mywebsite.com     >> )
Will send HTTP request, from 5 threads, to mywebsite.com, requesting "page.php" page.

===============================
|| CONTROLLING LOIC FROM IRC || (thanks to NewEraCracker)
===============================

As an OP, Admin or Owner, set the channel topic or send a message like the following:
!lazor targetip=127.0.0.1 message=test_test port=80 method=tcp wait=false random=true

To start an attack, type:
!lazor start

Or just append "start" to the END of the topic:
!lazor targetip=127.0.0.1 message=test_test port=80 method=tcp wait=false random=true start

To reset loic's options back to its defaults:
!lazor default

To stop an attack:
!lazor stop
and be sure to remove "start" from the END of the topic, if it exists, too.

Take a look at source code for more details.


Added for New LOIC : From IRC, you can define several targets, separated by ";" :
!lazor targetip=127.0.0.1;anurladdressalsoworks.fr;etc.com;

The NewLOIC "clients" will "choose" automatically a target in the list.


You can also set priority to some targets :
!lazor targetip=2,127.0.0.1;9,anurladdressalsoworks.fr;6,etc.com;

On average, if 17 newLOIC are running (example), 2 will take "127.0.0.1" as target, 6 will take "etc.com" as target, ... do you understand ? =)

	
I, Peacemaking, am not responsible for the use that you give to this tool.
You cannot blame me if you use this tool to attack servers you don't own and get caught.
This tool is released for educational purposes only and comes with no warranty at all.



10.08.2011 : Added a graphical (GTK) interface. You can now use NewLoic without any command line. (I compiled it only for Windows, at the moment, but code is portable)


TODO list :
- Clean the source code.
- Add multi-target mode.
- Learn english.
- Clean the source code.
- Add an hidden mode (Windows) or daemon mode (Linux).
- Add file support (config files, subsites/http_subsites/fake_IPs lists in files).
- Enable OverLord mode (like old .NET LOIC)
- Clean the source code.
- Clean the TODO list.

(sorry for the language, text will be revised soon)