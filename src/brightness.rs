use gtk::{
    gio::{DBusCallFlags, DBusConnection},
    glib::{self, g_warning, object::ObjectExt, variant::ToVariant, VariantTy},
    prelude::RangeExt,
    Builder, Label, Scale,
};

use crate::types::dbus;
use crate::types::HandlerError;

pub fn handle_brightness(builder: &Builder, conn: DBusConnection) -> Result<(), HandlerError> {
    let scale = builder
        .object::<Scale>("brightness-scale")
        .ok_or(HandlerError::ObjectError("Failed to get brightness-scale"))?;
    let label =
        builder
            .object::<Label>("brightness-scale-label")
            .ok_or(HandlerError::ObjectError(
                "Failed to get brightness-scale-label",
            ))?;

    let signal = scale.connect_value_changed(glib::clone!(
        #[weak]
        label,
        #[strong]
        conn,
        move |scale| {
            let value = scale.value() as u32;

            label.set_text(&format!("{}", value));

            let res = conn.call_future(
                dbus::BUS_NAME,
                dbus::Controllers::BRIGHTNESS,
                &dbus::Controllers::to_interface(dbus::Controllers::BRIGHTNESS),
                dbus::Methods::SET_BRIGHTNESS,
                Some(&(value,).to_variant()),
                None,
                DBusCallFlags::NONE,
                dbus::Timeout::NONE,
            );

            glib::MainContext::default().spawn_local(async move {
                let res = res.await;

                if let Err(e) = &res {
                    g_warning!(None, "DBus call error: {e:?}");
                }
            });
        }
    ));

    // Get brightness on startup
    let res = conn.call_future(
        dbus::BUS_NAME,
        dbus::Controllers::BRIGHTNESS,
        &dbus::Controllers::to_interface(dbus::Controllers::BRIGHTNESS),
        dbus::Methods::GET_BRIGHTNESS,
        None,
        Some(VariantTy::TUPLE),
        DBusCallFlags::NONE,
        dbus::Timeout::NONE,
    );
    glib::MainContext::default().spawn_local(glib::clone!(
        #[weak]
        label,
        #[weak]
        scale,
        async move {
            let res = res.await;

            if let Err(e) = &res {
                g_warning!(None, "DBus call error: {e:?}");
                return;
            }
            let Some(p) = res.unwrap().child_value(0).get::<u32>() else {
                g_warning!(None, "GetBrightness callback returned incorrect value");
                return;
            };

            label.set_text(&format!("{}", p));

            scale.block_signal(&signal);
            scale.set_value(p as f64);
            scale.unblock_signal(&signal);
        }
    ));

    Ok(())
}
