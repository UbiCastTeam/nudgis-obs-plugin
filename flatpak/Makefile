all: generate_flatpak

check_deps:
	@echo check
	@command -v flatpak-builder &> /dev/null || { echo >&2 "please install `flatpak-builder`"; exit 1; }
	@# add flathub
	@flatpak remote-add --if-not-exists --user flathub https://dl.flathub.org/repo/flathub.flatpakrepo

build: com.obsproject.Studio.Plugin.Nudgis.yml Makefile | check_deps
	@echo build
	@flatpak-builder --install-deps-from=flathub --repo=local_repo --force-clean build $<

generate_flatpak: build nudgis-obs-plugin.flatpak
	@echo "==> flatpak package ready: $<"

nudgis-obs-plugin.flatpak:
	@flatpak build-bundle --runtime local_repo $@ com.obsproject.Studio.Plugin.Nudgis stable

clean:
	@echo clean
	rm -rf *.flatpak build local_repo

.PHONY: clean generate_flatpak


