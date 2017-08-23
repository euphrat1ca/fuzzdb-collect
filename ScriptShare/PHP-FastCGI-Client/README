                       _________________________

                       PHP FastCGI Client README
                       _________________________


AUTHOR & CONTACT
================

Charron Pierrick
    - pierrick@adoy.net


DOCUMENTATION & DOWNLOAD
========================

Latest version is available on github at :
    - http://github.com/adoy/PHP-FastCGI-Client/

Documentation can be found on :
    - http://github.com/adoy/PHP-FastCGI-Client/


LICENSE
=======

This Code is released under the MIT license.

For the full copyright and license information, please view the LICENSE
file that was distributed with this source code.

How can I use it ?
==================

require 'vendor/autoload.php';

use Adoy\FastCGI\Client;

// Existing socket, such as Lighttpd with mod_fastcgi:
$client = new Client('unix:///path/to/php/socket', -1);
// Fastcgi server, such as PHP-FPM:
$client = new Client('localhost', '9000');
$content = 'key=value';
echo $client->request(
	array(
		'GATEWAY_INTERFACE' => 'FastCGI/1.0',
		'REQUEST_METHOD' => 'POST',
		'SCRIPT_FILENAME' => 'test.php',
		'SERVER_SOFTWARE' => 'php/fcgiclient',
		'REMOTE_ADDR' => '127.0.0.1',
		'REMOTE_PORT' => '9985',
		'SERVER_ADDR' => '127.0.0.1',
		'SERVER_PORT' => '80',
		'SERVER_NAME' => 'mag-tured',
		'SERVER_PROTOCOL' => 'HTTP/1.1',
		'CONTENT_TYPE' => 'application/x-www-form-urlencoded',
		'CONTENT_LENGTH' => strlen($content)
	),
	$content
);

Command line tool
=================

Run a call through a network socket:

    ./fcgiget.php localhost:9000/status

Run a call through a Unix Domain Socket

    ./fcgiget.php unix:/var/run/php-fpm/web.sock/status

This command line tool is provided for debuging purpose.
