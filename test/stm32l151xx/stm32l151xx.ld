/********************************************************************
* @author: Tien Ta
* @date  : 25/03/2026
********************************************************************/

ENTRY(Reset_Handler)

_estack = ORIGIN(RAM) + LENGTH(RAM);

MEMORY {
  RAM  (xrw): ORIGIN = 0x20000000, LENGTH = 16K
  FLASH(rx):  ORIGIN = 0x08000000, LENGTH = 64K
}

_stack_size = DEFINED(_stack_size) ? _stack_size : 2K;
_heap_size  = DEFINED(_heap_size)  ? _heap_size  : 1K;

SECTIONS {

  .vector :
  {
    . = ALIGN(4);
    KEEP(*(.vector))
    . = ALIGN(4);
  } > FLASH

  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)
    KEEP(*(.init))
    KEEP(*(.fini))
    . = ALIGN(4);
    _etext = .;
  } > FLASH

  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } > FLASH

  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } >FLASH

  .ARM :
  {
    . = ALIGN(4);
    __exidx_start = .;
    *(.ARM.exidx*)
    . = ALIGN(4);
    __exidx_end = .;
  } >FLASH

  .ARM.attributes :
  {
    . = ALIGN(4);
    *(.ARM.attributes)
    . = ALIGN(4);
  } > FLASH

  .preinit_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN(__preinit_array_start = .);
    KEEP(*(.preinit_array*))
    PROVIDE_HIDDEN(__preinit_array_end = .);
  } > FLASH

  .init_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN(*__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array*))
    PROVIDE_HIDDEN(__init_array_end = .);
  } > FLASH

  .fini_array :
  {
    PROVIDE_HIDDEN(__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array*))
    PROVIDE_HIDDEN(__fini_array_end = .);
  } > FLASH

  _sidata = LOADADDR(.data);

  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)
    . = ALIGN(4);

    _edata = .;
  } > RAM AT>FLASH

  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
  __bss_end__ = .;
  _ebss = __bss_end__;
  } > RAM

  .heap :
  {
    . = ALIGN(8);
    PROVIDE(end = _ebss);
    PROVIDE(_end = _ebss);
    . = . + _heap_size;
    PROVIDE(_eheap = .);
    . = . + _stack_size;
    PROVIDE(_sp = .);
    . = ALIGN(8);
  } > RAM
}
