use gtk::{
    gio::{self, DBusCallFlags, DBusConnection},
    glib::{self, g_warning, variant::ToVariant},
    prelude::RangeExt,
    Builder, Label, Scale,
};

use crate::DAEMON_NAME;

// TODO: type define errors
pub fn handle_brightness(builder: &Builder, conn: DBusConnection) -> Result<(), ()> {
    let Some(scale) = builder.object::<Scale>("brightness-scale") else {
        return Err(());
    };
    let Some(label) = builder.object::<Label>("brightness-scale-label") else {
        return Err(());
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
                Some(DAEMON_NAME),
                "com.vinii.BrightnessController",
                "com.vinii.BrightnessController",
                "SetBrightness",
                // TODO: may need to be a tuple
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
