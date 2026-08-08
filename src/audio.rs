use std::rc::Rc;

use gtk::{
    gio::{DBusCallFlags, DBusConnection},
    glib::{self, g_warning, object::ObjectExt, variant::ToVariant, SignalHandlerId, VariantTy},
    prelude::{RangeExt, WidgetExt},
    Box, Builder, GestureClick, Image, Label, Scale,
};

use crate::{types::HandlerError, ui::get_volume_icon};
use crate::{
    types::{dbus, Program},
    ui,
};

fn update_volume(
    variant: glib::Variant,
    label: Label,
    image: Image,
    scale: Scale,
    signal: Option<&SignalHandlerId>,
) {
    //if variant.n_children() == 0 {
    //    return;
    //}
    let percentage = variant.child_value(0).get::<u32>();
    let is_muted = variant.child_value(1).get::<bool>();

    match (percentage, is_muted) {
        (Some(percentage), Some(is_muted)) => {
            label.set_text(&format!("{}", percentage));
            image.set_icon_name(Some(&get_volume_icon(percentage, is_muted)));

            if let Some(signal) = signal {
                scale.block_signal(signal);
                scale.set_value(percentage as f64);
                scale.unblock_signal(signal);
            }
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
        #[weak]
        label_image,
        #[strong]
        scale,
        #[strong]
        conn,
        move |_| {
            let value = scale.value() as u32;

            let conn = conn.clone();
            let scale = scale.clone();
            glib::spawn_future_local(async move {
                let res = conn
                    .call_future(
                        Some(Program::BACKEND_NAME),
                        dbus::Controllers::AUDIO,
                        &dbus::Controllers::to_interface(dbus::Controllers::AUDIO),
                        dbus::Methods::SET_AUDIO,
                        Some(&(value,).to_variant()),
                        Some(VariantTy::TUPLE),
                        DBusCallFlags::NONE,
                        dbus::Timeout::NONE,
                    )
                    .await;
                match res {
                    Ok(v) => {
                        update_volume(v, label, label_image, scale, None);
                    }
                    Err(e) => g_warning!(None, "DBus call error: {e:?}"),
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
            match res {
                Ok(v) => {
                    update_volume(v, label, label_image, scale, Some(&shared_id));
                }
                Err(e) => g_warning!(None, "DBus call error: {e:?}"),
            }
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
                    match res {
                        Ok(..) => {
                            let get = conn
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
                            match get {
                                Ok(v) => {
                                    let is_muted = v.child_value(1).get::<bool>();
                                    match is_muted {
                                        Some(v) => {
                                            label_image.set_icon_name(Some(&get_volume_icon(0, v)))
                                        }
                                        None => g_warning!(None, "DBus unknown call error"),
                                    }
                                }
                                Err(e) => g_warning!(None, "DBus call error: {e:?}"),
                            }
                        }
                        Err(e) => g_warning!(None, "DBus call error: {e:?}"),
                    }
                }
            ));
        }
    ));
    label_box.add_controller(click_controller);

    Ok(())
}
