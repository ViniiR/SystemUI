use gtk::{
    gio::{DBusCallFlags, DBusConnection},
    glib::{self, g_warning},
    prelude::ButtonExt,
    Builder, Button,
};

use crate::types::HandlerError;
use crate::types::{dbus, Program};

pub fn handle_conservation(
    builder: &Builder,
    conn: DBusConnection,
) -> Result<(), HandlerError<'_>> {
    let conservation =
        builder
            .object::<Button>("conservation-mode-button")
            .ok_or(HandlerError::ObjectError(
                "Failed to get conservation-mode-button",
            ))?;

    // TODO: get current conservation status and update UI
    conservation.connect_clicked(move |_btn| {
        let res = conn.call_future(
            Some(Program::BACKEND_NAME),
            dbus::Controllers::CONSERVATION,
            &dbus::Controllers::to_interface(dbus::Controllers::CONSERVATION),
            dbus::Methods::TOGGLE_CONSERVATION,
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
