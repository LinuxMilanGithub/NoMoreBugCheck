#include <ntddk.h>

CHAR KeBugCheckExOrignalBytes[14] = { 0 };
CHAR KeBugCheckOrignalBytes[14] = { 0 };
ULONG_PTR KeBugCheckExAddress;
ULONG_PTR KeBugCheckAddress;

// Funktion zum Deaktivieren der Patch Guards
VOID DisablePatchGuard() {
    // CR0 Register modifizieren um Schreibschutz zu deaktivieren
    ULONG_PTR cr0;
    cr0 = __readcr0();
    cr0 &= ~0x10000;
    __writecr0(cr0);

    // MDL für Kernel Memory erstellen
    PMDL mdl = IoAllocateMdl((PVOID)KeBugCheckAddress, sizeof(KeBugCheckOrignalBytes), FALSE, FALSE, NULL);
    if (mdl) {
        MmBuildMdlForNonPagedPool(mdl);
        // Schreibschutz aufheben
        mdl->MdlFlags |= MDL_MAPPED_TO_SYSTEM_VA;
        MmProtectMdlSystemAddress(mdl, PAGE_EXECUTE_READWRITE);
        IoFreeMdl(mdl);
    }

    // CR0 Register wiederherstellen
    cr0 = __readcr0();
    cr0 |= 0x10000;
    __writecr0(cr0);
}

VOID DisablePatchGuardEx() {
    // CR0 Register modifizieren um Schreibschutz zu deaktivieren
    ULONG_PTR cr0;
    cr0 = __readcr0();
    cr0 &= ~0x10000;
    __writecr0(cr0);

    // MDL für Kernel Memory erstellen
    PMDL mdl = IoAllocateMdl((PVOID)KeBugCheckExAddress, sizeof(KeBugCheckExOrignalBytes), FALSE, FALSE, NULL);
    if (mdl) {
        MmBuildMdlForNonPagedPool(mdl);
        // Schreibschutz aufheben
        mdl->MdlFlags |= MDL_MAPPED_TO_SYSTEM_VA;
        MmProtectMdlSystemAddress(mdl, PAGE_EXECUTE_READWRITE);
        IoFreeMdl(mdl);
    }

    // CR0 Register wiederherstellen
    cr0 = __readcr0();
    cr0 |= 0x10000;
    __writecr0(cr0);
}

// Funktion zum Überschreiben von Speicherbereichen
NTSTATUS Overwrite(PVOID Address, PVOID Data, ULONG Size) {
    PHYSICAL_ADDRESS PhysAddress = MmGetPhysicalAddress(Address);
    PVOID MappedAddress = MmMapIoSpace(PhysAddress, Size, MmNonCached);

    if (MappedAddress == NULL)
        return STATUS_INSUFFICIENT_RESOURCES;

    RtlCopyMemory(MappedAddress, Data, Size);
    MmUnmapIoSpace(MappedAddress, Size);
    return STATUS_SUCCESS;
}

// Rest des Codes bleibt unverändert...
VOID KeHookedBugCheckEx(ULONG BugCheckCode, ULONG_PTR Code1, ULONG_PTR Code2,
    ULONG_PTR Code3, ULONG_PTR Code4) {
    DbgPrint("[*] KeBugCheckEx abgefangen: Prozess %d, Thread %d\n",
        PsGetCurrentProcessId(), PsGetCurrentThreadId());
    DbgPrint("[*] KeBugCheckEx(0x%llx, 0x%llx, 0x%llx, 0x%llx)\n",
        BugCheckCode, Code1, Code2, Code3, Code4);
    DbgPrint("[*] BugCheckEx ignoriert. Ausführung wird fortgesetzt.\n");
}

VOID KeHookedBugCheck(ULONG BugCheckCode) {
    DbgPrint("[*] KeBugCheck abgefangen: Prozess %d, Thread %d\n",
        PsGetCurrentProcessId(), PsGetCurrentThreadId());
    DbgPrint("[*] KeBugCheck(0x%lx)\n", BugCheckCode);
    DbgPrint("[*] BugCheck ignoriert. Ausführung wird fortgesetzt.\n");
}

VOID DriverUnload(PDRIVER_OBJECT DriverObject) {
    UNREFERENCED_PARAMETER(DriverObject);
    Overwrite((PVOID)KeBugCheckExAddress, KeBugCheckExOrignalBytes, sizeof(KeBugCheckExOrignalBytes));
    Overwrite((PVOID)KeBugCheckAddress, KeBugCheckOrignalBytes, sizeof(KeBugCheckOrignalBytes));
    DbgPrint("[*] Treiber entladen und Originalbytes wiederhergestellt.\n");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(RegistryPath);

    DbgPrint("[*] Treiber geladen.\n");
    DriverObject->DriverUnload = DriverUnload;

    DisablePatchGuardEx();
    KeBugCheckExAddress = (ULONG_PTR)KeBugCheckEx;
    RtlCopyMemory(KeBugCheckExOrignalBytes, (PVOID)KeBugCheckExAddress, sizeof(KeBugCheckExOrignalBytes));
    Overwrite((PVOID)KeBugCheckExAddress, (PVOID)KeHookedBugCheckEx, sizeof(KeBugCheckExOrignalBytes));

    DisablePatchGuard();
    KeBugCheckAddress = (ULONG_PTR)KeBugCheck;
    RtlCopyMemory(KeBugCheckOrignalBytes, (PVOID)KeBugCheckAddress, sizeof(KeBugCheckOrignalBytes));
    Overwrite((PVOID)KeBugCheckAddress, (PVOID)KeHookedBugCheck, sizeof(KeBugCheckOrignalBytes));

    DbgPrint("[*] KeBugCheckEx und KeBugCheck erfolgreich gehookt.\n");

    return STATUS_SUCCESS;
}