all:
	$(MAKE) -C efi
	$(MAKE) -C gen
	$(MAKE) -C i386/gen
	$(MAKE) -C i386/math
	$(MAKE) -C locale
	$(MAKE) -C nls
	$(MAKE) -C regex
	$(MAKE) -C stdio
	$(MAKE) -C stdlib
	$(MAKE) -C stdtime
	$(MAKE) -C string
	$(MAKE) -C sys
	$(MAKE) -C wchar

clean:
	$(MAKE) -C efi clean
	$(MAKE) -C gen clean
	$(MAKE) -C i386/gen clean
	$(MAKE) -C i386/math clean
	$(MAKE) -C locale clean
	$(MAKE) -C nls clean
	$(MAKE) -C regex clean
	$(MAKE) -C stdio clean
	$(MAKE) -C stdlib clean
	$(MAKE) -C stdtime clean
	$(MAKE) -C string clean
	$(MAKE) -C sys clean
	$(MAKE) -C wchar clean
	rm -f libc.a
