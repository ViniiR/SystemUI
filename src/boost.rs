use gtk::{
    gio::{DBusCallFlags, DBusConnection},
    glib::{self, g_warning},
    prelude::ButtonExt,
    Builder, Button,
};

use crate::types::HandlerError;
use crate::types::{dbus, Program};

pub fn handle_boost(builder: &Builder, conn: DBusConnection) -> Result<(), HandlerError<'_>> {
    let boost = builder
        .object::<Button>("boost-mode-button")
        .ok_or(HandlerError::ObjectError("Failed to get boost-mode-button"))?;

    // TODO: get current boost status and update UI
    boost.connect_clicked(move |_btn| {
        let res = conn.call_future(
            Some(Program::BACKEND_NAME),
            dbus::Controllers::BOOST,
            &dbus::Controllers::to_interface(dbus::Controllers::BOOST),
            dbus::Methods::TOGGLE_BOOST,
            None,
            None,
            DBusCallFlags::NONE,
            dbus::Timeout::NONE,
        );

        glib::spawn_future_local(async move {
            let res = res.await;

            if let Err(e) = &res {
                g_warning!(None, "DBus call error: {e:?}");
            }
        });
    });

    Ok(())
}
