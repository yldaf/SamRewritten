#pragma once

#include <gtk/gtk.h>

// Ubuntu 18.04 only ships gtkmm 3.22, which doesn't define Gtk::make_managed.
// But it's easy enough to define it if it's not defined

#if (GTK_MAJOR_VERSION < 3)
    #error "Samrewritten cannot compile on GTK < 3"
#elif (GTK_MAJOR_VERSION == 3)
#if (GTK_MINOR_VERSION < 24)
// Best effort..it is unkown if it will compile on <22
namespace Gtk
{

template<class T, class... T_Args>
T* make_managed(T_Args&&... args)
{
  return manage(new T(std::forward<T_Args>(args)...));
}

}
#endif
#endif // GTK_MAJOR_VERSION
// Hopefully there's forward compatibility
