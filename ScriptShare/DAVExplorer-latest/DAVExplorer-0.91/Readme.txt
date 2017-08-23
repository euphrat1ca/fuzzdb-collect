1. OVERVIEW

DAV Explorer is a WebDAV and DeltaV client application that uses the WebDAV,
DeltaV and ACL protocols to provide:
     A tree view of a WebDAV server 
     Upload and download of Web resources 
     Locking and unlocking of resources for collaboration support 
     Display all resource properties, or just lock properties 
     Copying collections and individual resources 
     Renaming of individual resources 
     Creating new collections 
     Delete collections or individual resources 
     Logging of protocol activity 
     Creating new versions of resources
     Checking out and checking in of resources
     Listing of resource versions
     Listing of Access Control information
     Adding and Modification of Access Control Lists
     ACL Reports

The user interface for DAV Explorer is similar in look and functionality
to the Explorer program which is provided by the Windows operating system.
DAV Explorer is a useful tool for interoperability testing a WebDAV server,
since it is capable of exercising the majority of the functionality
specified in RFC 2518, the WebDAV Distributed Authoring Protocol
specification, while logging the protocol stream. However, DAV Explorer can
also be used for remote namespace management, and has collaboration support
for groups which employ a lock-download-work-upload-unlock authoring
process.
DAV Explorer also supports the linear versioning of resources through the
use of RFC 3253, the DeltaV Versioning Extensions to WebDAV protocol specification.
DAV Explorer supports the linear checkout, uncheckout, checkin, and reporting
functionality specified in the DeltaV specification.
DAV Explorer also supports the WebDAV Access Control Protocol as specified
in RFC 3744.

DAV Explorer is a Java application which uses Java 2 and has successfully
been run on Microsoft Windows from Windows 95 to Server 2003,
Solaris and Linux.
It has been reported to run on Macintosh computers, with MacOS 9 and
MRJ 2.2.3, and with Mac OS X.
DAV Explorer may run on other platforms, but this has not been verified.

DAV Explorer also works as applet. Usage:
<APPLET	ARCHIVE="DAVExplorer.jar"
        CODE="edu.uci.ics.DAVExplorer.AppletMain.class"
        WIDTH=800
        HEIGHT=400>
        <PARAM NAME=uri VALUE="http://dav.somewhere.com/webdav/">
        <PARAM NAME=username VALUE="username">
        <PARAM NAME=password VALUE="password">
        <PARAM NAME=proxy VALUE="proxy.com">
</APPLET>
Alternative Usage:
<EMBED TYPE     = "application/x-java-applet"
	   WIDTH    = "800"
	   HEIGHT   = "400"
	   code     = "edu.uci.ics.DAVExplorer.AppletMain.class"
	   archive  = "DAVExplorer.jar"
	   uri      = "http://dav.somewhere.com/webdav/"
	   username = "username"
	   password = "password"
       proxy    = "proxy.com">
</EMBED>
The username and password parameters are optional for security reasons.
In fact, we consider it BAD PRACTICE to provide them in clear text on a
webpage, unless the webpage is accessible only through secure means, e.g.,
using SSL.
If they are not specified on the webpage, they are requested interactively.
The proxy parameter is optional and allows the automatic use of a proxy
server if specified.
The applet code also supports the use of SSL. It has been tested with JRE 1.4.
Since DAV Explorer accesses restricted properties, the jar file is signed by
the DAV Explorer Team to allow the use as applet.


2. LICENSE

DAV Explorer is released under an Apache-style license. See the file
License.txt in the distribution for details.


3. SOURCE CODE

The DAV Explorer source files can be downloaded from
http://www.ics.uci.edu/~webdav/download.html.

The source files are also available via Subversion at
http://louvre.ics.uci.edu:8080/svn/DAVExplorer/
To retrieve the complete source tree, check out the trunk directory with your
favorite Subversion client tool.
The command line format:
svn checkout http://louvre.ics.uci.edu:8080/svn/DAVExplorer/trunk DAVExplorer
Anonymous access is read-only, please submit code patches to <dav-exp@ics.uci.edu>. 
NOTE: the archive location may change in the near future, so be sure to check
the DAV Explorer download website http://www.ics.uci.edu/~webdav/download.html
for any changes.

If the DAV Explorer jar file is created from the source, it has to be
signed with the jarsigner tool before it can run as applet. The jar file
included in the binary distribution is signed by the DAV Explorer Team.

4. COMMAND LINE OPTIONS

DAV Explorer accepts the following command line options:

-Dhelp, -Dhelp=yes
  Prints a list of all options.
  
-Ddebug=option
  where option is one of:
  all          all function traces are enabled
  request      function traces related to HTTP requests are enabled
  response     function traces related to HTTP responses are enabled
  treeview     function traces related to the tree view on the left
  		side of the DAVExplorer window are enabled
  treenode     function traces related to each node in the tree view
  		are enabled
  fileview     function traces related to the file view on the right
  		side of the DAVExplorer window are enabled

-Dpropfind=allprop
  This option results in using the <allprop> tag in PROPFIND.
  
-DSSL=yes
  This option enables the use of SSL.
  NOTE: SSL support can also be enabled through an entry in the Edit menu.
  
-DSharePoint=yes
  This option enables a workaround for a bug in Microsoft's SharePoint
  server which allows tags to start with a digit.
  
-DApache=yes
  This option enables a workaround for a bug in Apache 1.3.x, which returns
  a 500 error in response to a PROPPATCH if the Host: header contains a
  port number.

-Dlocal=no
  This option prevents showing the local directory structure in the
  main DAV Explorer window.

-Dcompress=no
  This option prevents DAV Explorer from accepting compressed data.
  This is a workaround for a bug in MS Exchange that results in corrupt
  compressed data.


5. Contributors
The file contributors.txt contains a list of all contributors.
Thanks to everybody.


6. CHANGELOG

Changes for version 0.91:
- Re-Enabled authentication requests for PUT, broken by changing to streaming
  PUT data in 0.82 (thanks to Marc Richard for alerting us to the problem.)
- Added support for proxy entry in applet settings (thanks to Lu?s Ferreira
  for the suggestion.)
- Fixed usage of webpage-provided username/password for applet usage.
- Added workaround for Documentum Modified-Date bug (their date strings
  are localized, which violates RFC 2616) (thanks to Holger Spalt for
  alerting us to the problem.)
- Integrated John Barton's refactoring changes and drop support.
- Added the -Dcompress option to disable accepting compressed data.
  This is a workaround for a bug in MS Exchange that results in corrupt
  compressed data (thanks to Fr?d?ric Esnouf for alerting us to the problem.)
- Added handling of ISO 8601 Date strings. This is a workaround for a bug
  in MS Exchange, whose date strings in the <getlastmodified> tag uses the
  ISO 8601 format instead of the RFC 2616 format required by the WebDAV
  RFC (thanks to Fr?d?ric Esnouf for alerting us to the problem.)
- Fixed the charset handling and unencoding of data from the server.
- Added Edit menu option to select SSL. The selection is persisted.
- Added ACL (RFC 3744) support.

Changes for version 0.90:
- Bug fix in HTTPClient for PUT with Stream and Digest authentication.
- Right view now works when columns are reorganized.
- Right view now allows sorting on every column.
- Basic DeltaV support: version-control, checkout, uncheckout, checkin,
  report.

Changes for version 0.82:
- Made sure that DAV Explorer runs with JDK 1.1.x, for MacOS 9 compatibility
- Updated to run with JDK 1.4
- Incorporated Karen Schuchardt's changes to improve the loading of images
- Added the -Dlocal option to disable reading and showing the local
  directory structure.
- Fixed the icon locator code to account for drive letters on Windows.
- Special handling of PUT to support files > 2GB.
- Added Translate header to better support IIS.
- Incorporated Brian Johnson's changes for applet support.
- Incorporated Thoralf Rickert's progress bar and persistent URL support.
- Lock info defaults to "DAV Explorer".
- Added support for shared locks.

Changes for version 0.81:
- Fixed handling of default namespace in the view/modify property dialog
- Fixed menu selection of view/modify properties dialog
- Fixed problems with property addition to root
- Apache 1.3.x workaround: PROPPATCH returns a 500 error if the Host: header
  contains the port number. The workaround is activated with the option
  -DApache=yes
- Made the MS SharePoint workaround optional. It is activated with the
  following command line: java -jar -DSharePoint=true DAVExplorer.jar
- Improved handling of non-ASCII UTF-8 characters.
- Improved the logging of chunked data.
- Now allowing untrusted certificates by presenting a choice to the user.
- Fixed problems with modifying nested properties.
- Unified option selection to yes/no.

Changes for version 0.80:
- Makefile for HTTPClient now works with JSSE installed as "bundled"
  extension.
- Fixed handling of EOF for whitespace after the final tag (interoperability
  bug with Adobe InScope).
- Changed the parser code to allow tag names starting with a digit
  (interoperability problem with Microsoft SharePoint (a bug in SharePoint)).
- Removed whitespace in the created XML for lockowner and keepalive
  properties.
- Interoperability problem fixed for cases when properties for a collection
  and its contents are requested and the server doesn't send properties for
  the collection itself.
- Modified the authentication code to make use of the HTTPClient
  functionality for Digest authentication.
- Copy and Move now allow entering the target, making them more flexible.
- For initial contact with a server, we now send an OPTIONS request.
- Rewrite of the View Property dialog.
- PROPPATCH support implemented (finally).
- The package name of the main DAVExplorer files was changed to
  edu.uci.ics.DAVExplorer.

Changes for version 0.72:
- Support for operation through proxy servers
- Update to HTTPClient 0.3.3

Changes for version 0.71:
- Fixed bug in HTTPClient that prevented proper handling of authentication in
  case the server allows multiple possible authentication headers (thanks to
  Thierry Janaudy for alerting us to this problem).
- Fixed broken handling of & in filenames in the MS parser (thanks to Dennis
  Craig for alerting us to this problem).
- Now using the standard https port (443) for requests with SSL.
- Closing the properties dialog with the close button does not exit the
  application anymore.

Changes for version 0.70:
- Support for SSL (https protocol)
  The SSL package supported is Sun's Java Secure Socket Extensions 1.0.2
  (JSSE), available at http://java.sun.com/products/jsse/
  SSL is activated with the following command line:
  java -jar -Dssl=true DAVExplorer.jar
  If a self-certified certificate is used, the certificate has to be added
  to the JSSE keystore with the keytool program from the Java JDK.
  On MS Windows, JSSE ignores the default keystore, instead, the certificate
  has to be stored in a particular JSSE keystore located in the JRE directory
  tree, usually at JAVA_HOME/jre/lib/security/jssecacerts
- Compatibility with JDK 1.3 was confirmed. The DAVExplorer.jar file is now
  created with JDK 1.3.0
- Interoperability problem (another SiblingEnumeration bug in the parser)
  with IIS 5.0 fixed (thanks to Ron Gutfinger for alerting us to this
  problem).

Changes for version 0.62:
- Improved recovery after server timeout

Changes for version 0.61:
- Added check for CDATA to improve interoperability for Sharemation's server.
- Changed the enumeration in parseResponse() to SiblingEnumeration to
  avoid parsing the wrong href tag (thanks to Michelle Harris for
  alerting us to this problem).
- Fixed string comparison in case of multiple <propstat> tags.
- href tag is now stripped from lock owner string (thanks to Eric Giguere
  for submitting this fix).

Changes for version 0.60:
- Fixed logging of incoming data.
- The DAV4J workaround (see below in the changes for version 0.57) is also
  required for locking support with mod_dav up to version 0.9.16.

Changes for version 0.59:
- Interoperability problem with www.sharemation.com fixed.
- The authentication dialog now acts as Authentication Handler for
  HTTPClient.

Changes for version 0.58:
- Fix interoperability problem with Glyphica server:
  The Glyphica server returns a wrong locktoken field, and DAV Explorer
  did not react to this properly.

Changes for version 0.57:
- Workaround for interoperability with DAV4J
  The workaround is activated with the following command line:
  java -jar -Dpropfind=allprop DAVExplorer.jar

Changes for version 0.56:
- DAVExplorer is now instrumented for function trace output.
  Function trace output is enabled by running DAVExplorer with the following
  command line:
  java -jar -Ddebug=option DAVExplorer.jar
  where option is one of:
  all                   all function traces are enabled
  request               function traces related to HTTP requests are enabled
  response              function traces related to HTTP responses are enabled
  treeview              function traces related to the tree view on the left
                        side of the DAVExplorer window are enabled
  treenode              function traces related to each node in the tree view
                        are enabled
  fileview              function traces related to the file view on the right
                        side of the DAVExplorer window are enabled
  The trace output is written to stderr.
