use std::rc::Rc;

use gtk::{
    gio::{DBusCallFlags, DBusConnection},
    glib::{self, g_warning, object::ObjectExt, variant::ToVariant, SignalHandlerId, VariantTy},
    prelude::{RangeExt, WidgetExt},
    Box, Builder, GestureClick, Image, Label, Scale,
};

use crate::types::{dbus, Program};
use crate::{types::HandlerError, ui::get_volume_icon};

async fn update_volume(
    conn: DBusConnection,
    label: Label,
    image: Image,
    scale: Scale,
    signal: &SignalHandlerId,
) {
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
            image.set_icon_name(Some(&get_volume_icon(percentage, is_muted)));

            scale.block_signal(signal);
            scale.set_value(percentage as f64);
            scale.unblock_signal(signal);
        }
        _ => {
            g_warning!(None, "GetAudio callback returned invalid tuple types");
        }
    }
}

pub fn handle_audio(builder: &Builder, conn: DBusConnection) -> Result<(), HandlerError<'_>> {
    let scale = builder
        .object::<Scale>("volume-scale")
        .ok_or(HandlerError::ObjectError("Failed to get volume-scale"))?;
    let label = builder
        .object::<Label>("volume-scale-label")
        .ok_or(HandlerError::ObjectError(
            "Failed to get volume-scale-label",
        ))?;
    let label_box =
        builder
            .object::<Box>("volume-scale-label-box")
            .ok_or(HandlerError::ObjectError(
                "Failed to get volume-scale-label-box",
            ))?;
    let label_image =
        builder
            .object::<Image>("volume-scale-label-image")
            .ok_or(HandlerError::ObjectError(
                "Failed to get volume-scale-label-image",
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
    let shared_id = Rc::new(signal);

    // Get volume on startup
    glib::spawn_future_local(glib::clone!(
        #[weak]
        label,
        #[weak]
        scale,
        #[strong]
        conn,
        #[strong]
        label_image,
        #[strong]
        shared_id,
        async move {
            update_volume(conn, label, label_image, scale, &shared_id).await;
        }
    ));

    let click_controller = GestureClick::new();
    click_controller.connect_pressed(glib::clone!(
        #[weak]
        label,
        #[weak]
        scale,
        #[strong]
        conn,
        #[strong]
        label_image,
        #[strong]
        shared_id,
        move |_gesture, _n_press, _x, _y| {
            glib::spawn_future_local(glib::clone!(
                #[strong]
                label_image,
                #[strong]
                conn,
                #[strong]
                shared_id,
                async move {
                    let res = conn
                        .call_future(
                            Some(Program::BACKEND_NAME),
                            dbus::Controllers::AUDIO,
                            &dbus::Controllers::to_interface(dbus::Controllers::AUDIO),
                            dbus::Methods::TOGGLE_AUDIO_MUTED,
                            None,
                            None,
                            DBusCallFlags::NONE,
                            dbus::Timeout::NONE,
                        )
                        .await;
                    if let Err(e) = &res {
                        g_warning!(None, "DBus call error: {e:?}");
                        return;
                    }

                    update_volume(conn, label, label_image, scale, &shared_id).await;
                }
            ));
        }
    ));
    label_box.add_controller(click_controller);

    Ok(())
}
