use gtk::{
    gio::{self, DBusCallFlags, DBusConnection},
    glib::{self, g_warning, variant::ToVariant, VariantTy, VariantType},
    prelude::RangeExt,
    Builder, Label, Scale,
};

use crate::types::dbus;
use crate::types::HandlerError;

// TODO: type define errors
pub fn handle_brightness(builder: &Builder, conn: DBusConnection) -> Result<(), HandlerError> {
    let Some(scale) = builder.object::<Scale>("brightness-scale") else {
        return Err(HandlerError::ObjectError);
    };
    let Some(label) = builder.object::<Label>("brightness-scale-label") else {
        return Err(HandlerError::ObjectError);
    };

    // IIFE
    (glib::clone!(
        #[strong]
        conn,
        #[strong]
        label,
        #[strong]
        scale,
        move || {
            let res = conn.call_sync(
                dbus::BUS_NAME,
                dbus::Controllers::BRIGHTNESS,
                &dbus::Controllers::to_interface(dbus::Controllers::BRIGHTNESS),
                dbus::Methods::GET_BRIGHTNESS,
                None,
                Some(VariantTy::TUPLE),
                DBusCallFlags::NONE,
                dbus::Timeout::NONE,
                gio::Cancellable::NONE,
            );

            if let Err(e) = &res {
                g_warning!(None, "DBus call error: {e:?}");
                return;
            }
            let Some(p) = res.unwrap().child_value(0).get::<u32>() else {
                g_warning!(None, "GetBrightness callback returned incorrect value");
                return;
            };
            label.set_text(&p.to_string());
            scale.set_value(p as f64);
        }
    ))();

    scale.connect_value_changed(glib::clone!(
        #[strong]
        conn,
        #[strong]
        label,
        move |scale| {
            let value = scale.value() as u32;

            // TODO: maybe should be inside callback
            label.set_text(&value.to_string());

            conn.call(
                dbus::BUS_NAME,
                dbus::Controllers::BRIGHTNESS,
                &dbus::Controllers::to_interface(dbus::Controllers::BRIGHTNESS),
                dbus::Methods::SET_BRIGHTNESS,
                Some(&(value,).to_variant()),
                None,
                DBusCallFlags::NONE,
                dbus::Timeout::NONE,
                gio::Cancellable::NONE,
                |res| {
                    if let Err(e) = &res {
                        g_warning!(None, "DBus call error: {e:?}");
                    }
                },
            );
        }
    ));

    Ok(())
}
