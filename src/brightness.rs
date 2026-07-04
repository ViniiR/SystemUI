use gtk::{
    gio::{self, DBusCallFlags, DBusConnection},
    glib::{self, g_warning, variant::ToVariant},
    prelude::RangeExt,
    Builder, Label, Scale,
};

use crate::{
    types::{DBusObjects, HandlerError},
    DAEMON_NAME, DBUS_INTERFACE,
};

// TODO: type define errors
pub fn handle_brightness(builder: &Builder, conn: DBusConnection) -> Result<(), HandlerError> {
    let Some(scale) = builder.object::<Scale>("brightness-scale") else {
        return Err(HandlerError::ObjectError);
    };
    let Some(label) = builder.object::<Label>("brightness-scale-label") else {
        return Err(HandlerError::ObjectError);
    };

    scale.connect_value_changed(glib::clone!(
        #[strong]
        conn,
        #[strong]
        label,
        move |scale| {
            let value = scale.value() as i32;

            label.set_text(&value.to_string());

            conn.call(
                DAEMON_NAME,
                DBusObjects::BrightnessController.as_str(),
                DBUS_INTERFACE,
                // TODO: type define these values
                "SetBrightness",
                Some(&value.to_variant()),
                None,
                DBusCallFlags::NONE,
                -1,
                gio::Cancellable::NONE,
                |res| {
                    if let Err(e) = res {
                        g_warning!(None, "DBus call error: {:?}", e);
                    }
                },
            );
        }
    ));

    Ok(())
}
