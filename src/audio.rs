use gtk::{
    gio::{DBusCallFlags, DBusConnection},
    glib::{self, g_warning, object::ObjectExt, variant::ToVariant, VariantTy},
    prelude::RangeExt,
    Builder, Label, Scale,
};

use crate::types::HandlerError;
use crate::types::{dbus, Program};

pub fn handle_audio(builder: &Builder, conn: DBusConnection) -> Result<(), HandlerError<'_>> {
    let scale = builder
        .object::<Scale>("volume-scale")
        .ok_or(HandlerError::ObjectError("Failed to get volume-scale"))?;
    let label = builder
        .object::<Label>("volume-scale-label")
        .ok_or(HandlerError::ObjectError(
            "Failed to get volume-scale-label",
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
                Some(Program::BACKEND_NAME),
                dbus::Controllers::AUDIO,
                &dbus::Controllers::to_interface(dbus::Controllers::AUDIO),
                dbus::Methods::SET_AUDIO,
                Some(&(value,).to_variant()),
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
        }
    ));

    // Get volume on startup
    glib::spawn_future_local(glib::clone!(
        #[weak]
        label,
        #[weak]
        scale,
        async move {
            let res = conn
                .call_future(
                    Some(Program::BACKEND_NAME),
                    dbus::Controllers::AUDIO,
                    &dbus::Controllers::to_interface(dbus::Controllers::AUDIO),
                    dbus::Methods::GET_AUDIO,
                    None,
                    Some(VariantTy::TUPLE),
                    DBusCallFlags::NONE,
                    dbus::Timeout::NONE,
                )
                .await;

            if let Err(e) = &res {
                g_warning!(None, "DBus call error: {e:?}");
                return;
            }
            let res = res.unwrap();

            let percentage = res.child_value(0).get::<u32>();
            let is_muted = res.child_value(1).get::<bool>();

            match (percentage, is_muted) {
                (Some(percentage), Some(is_muted)) => {
                    label.set_text(&format!("{}", percentage));

                    // TODO: set is_muted on ui, and make it clickable
                    scale.block_signal(&signal);
                    scale.set_value(percentage as f64);
                    scale.unblock_signal(&signal);
                }
                _ => {
                    g_warning!(None, "GetAudio callback returned invalid tuple types");
                }
            }
        }
    ));

    Ok(())
}
