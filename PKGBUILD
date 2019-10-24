# Maintainer: telans <telans@protonmail.com>
# Co-Maintainer: Simon Tas <simon.tas.st@gmail.com>

pkgname="samrewritten-git"
_pkgname="SamRewritten"
pkgver=r67.63761a7
pkgrel=1
pkgdesc="A Steam Achievement Manager For Linux."
arch=("any")
url="https://github.com/PaulCombal/SamRewritten"
license=("GPL3")
depends=("steam" "yajl" "gtk3" "glibc")
makedepends=("git" "make" "gcc")
source=("git+https://github.com/PaulCombal/SamRewritten.git")
sha256sums=("SKIP")

pkgver() {
    cd ${_pkgname}
    printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

build() {
    cd ${_pkgname}
    make
}

package() {  
    install -dm755 "${pkgdir}/usr/lib/"
    # Only copy required files. (Except for Glade files, as more may be added in the future.)
    cp -r --parents ${_pkgname}/{LICENSE,README.MD,bin/{launch.sh,libsteam_api.so,samrewritten},glade/*.glade,assets} "${pkgdir}/usr/lib/"
    # Executable
    install -dm755 "${pkgdir}/usr/bin"
    ln -s "/usr/lib/${_pkgname}/bin/launch.sh" "${pkgdir}/usr/bin/samrewritten"
    # Desktop Entry
    install -Dm644 "samrewritten.desktop" "${pkgdir}/usr/share/applications/samrewritten.desktop"
} 