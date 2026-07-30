use std::thread::spawn;

use gtk::{
    gio::{DBusCallFlags, DBusConnection},
    glib::{self, clone, g_warning, VariantTy},
    Builder, Image, Label,
};

use crate::types::{dbus, HandlerError, Program};

pub fn handle_battery(builder: &Builder, conn: DBusConnection) -> Result<(), HandlerError<'_>> {
    let label = builder
        .object::<Label>("battery-button-box-label")
        .ok_or(HandlerError::ObjectError("Failed to get battery label"))?;
    let image = builder
        .object::<Image>("battery-button-box-image")
        .ok_or(HandlerError::ObjectError("Failed to get battery image"))?;

    async fn make_handler(label: Label, image: Image, conn: DBusConnection) {
        let res = conn
            .call_future(
                Some(Program::BACKEND_NAME),
                dbus::Controllers::BATTERY,
                &dbus::Controllers::to_interface(dbus::Controllers::BATTERY),
                dbus::Methods::GET_BATTERY,
                None,
                Some(VariantTy::TUPLE), // "su"
                DBusCallFlags::NONE,
                dbus::Timeout::NONE,
            )
            .await;

        if let Err(e) = &res {
            g_warning!(None, "DBus call error: {e:?}");
            return;
        }
        let res = res.unwrap();

        let Some(icon_name) = res.child_value(0).get::<String>() else {
            g_warning!(None, "GetBattery callback returned incorrect first value");
            return;
        };
        let Some(percentage) = res.child_value(1).get::<u32>() else {
            g_warning!(None, "GetBattery callback returned incorrect second value");
            return;
        };

        label.set_text(&format!("{percentage}%"));
        image.set_icon_name(Some(&icon_name));
    }

    glib::spawn_future_local(make_handler(label.clone(), image.clone(), conn.clone()));

    glib::timeout_add_seconds_local(1, move || {
        glib::spawn_future_local(make_handler(label.clone(), image.clone(), conn.clone()));
        glib::ControlFlow::Continue
    });

    Ok(())
}
