<div align="center">
    <div>
        <img style="width: 10%;" src="https://githacks.org/_xeroxz/PTM/-/raw/ae420f13bf79069246ea2076d0926692aeb21562/img/PTM-icon.png"/>
    </div>
    <img src="https://githacks.org/xerox/ptm/-/raw/d9e67ce6baae8a455b2f9218fed8f1d8fca04a16/img/Windows%2010-supported-green.svg"/>
    <img src="https://githacks.org/xerox/ptm/-/raw/d9e67ce6baae8a455b2f9218fed8f1d8fca04a16/img/VDM-Inherited-green.svg"/>
</div>

# PTM - Paging Table Manipulation From Usermode

paging table manipulation from user-mode. operations such as getting and setting all paging table entries and values are offered. the code is aware of large pages. Link to write up can be found [here](https://back.engineering/01/12/2020/).

# example

```cpp
vdm::read_phys_t _read_phys = 
	[&](void* addr, void* buffer, std::size_t size) -> bool
{
	return vdm::read_phys(addr, buffer, size);
};

vdm::write_phys_t _write_phys =
	[&](void* addr, void* buffer, std::size_t size) -> bool
{
	return vdm::write_phys(addr, buffer, size);
};

vdm::vdm_ctx vdm(_read_phys, _write_phys);
nasa::mem_ctx my_proc(vdm);

const auto ntoskrnl_base =
    reinterpret_cast<void*>(
	    util::get_kmodule_base("ntoskrnl.exe"));

const auto ntoskrnl_pde = my_proc.get_pde(ntoskrnl_base);
std::printf("[+] pde.present -> %d\n", ntoskrnl_pde.second.present);
std::printf("[+] pde.pfn -> 0x%x\n", ntoskrnl_pde.second.pfn);
std::printf("[+] pde.large_page -> %d\n", ntoskrnl_pde.second.large_page);
```

```
[+] pde.present -> 1
[+] pde.pfn -> 0x10400
[+] pde.large_page -> 1
[+] press any key to close...
```

# table entry manipulation
- get/set pml4e's
- get/set pdpte's
- get/set pde's
- get/set pte's

# table manipulation
- copy table
- make self referencing table.

# virtual memory
- convert virtual addresses to physical addresses
- get table entries for a given address
- change table entries for a given address

# credit 
- buck#0001 - inspiration for most of this.
- Ch40zz - helping me connect the dots.

# related work
* [PSKP](https://githacks.org/_xeroxz/PSKP)
* [PTEditor](https://github.com/misc0110/PTEditor)
* [PSKDM](https://githacks.org/_xeroxz/PSKDM)
* [reverse-injector](https://githacks.org/_xeroxz/reverse-injector)
* [pclone](https://githacks.org/_xeroxz/pclone)