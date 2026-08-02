use gtk::{
    gio::{DBusCallFlags, DBusConnection},
    glib::{self, g_warning, VariantTy},
    prelude::{ButtonExt, WidgetExt},
    Builder, Button,
};

use crate::types::HandlerError;
use crate::types::{dbus, Program};

pub fn handle_boost(builder: &Builder, conn: DBusConnection) -> Result<(), HandlerError<'_>> {
    let boost = builder
        .object::<Button>("boost-mode-button")
        .ok_or(HandlerError::ObjectError("Failed to get boost-mode-button"))?;

    // TODO: get current boost status and update UI at startup!!!!!!!!!!!!!!!!!!!!
    boost.connect_clicked(move |btn| {
        let res = conn.call_future(
            Some(Program::BACKEND_NAME),
            dbus::Controllers::BOOST,
            &dbus::Controllers::to_interface(dbus::Controllers::BOOST),
            dbus::Methods::TOGGLE_BOOST,
            None,
            Some(VariantTy::TUPLE),
            DBusCallFlags::NONE,
            dbus::Timeout::NONE,
        );

        glib::spawn_future_local(glib::clone!(
            #[strong]
            btn,
            async move {
                let res = res.await;

                match &res {
                    Ok(_v) if let Some(b) = _v.child_value(0).get::<bool>() => {
                        const ACTIVE: &str = "button-active";
                        const INACTIVE: &str = "button-inactive";

                        btn.add_css_class(if b { ACTIVE } else { INACTIVE });
                        btn.remove_css_class(if b { INACTIVE } else { ACTIVE });
                    }
                    e => g_warning!(None, "DBus call error: {e:?}"),
                };
            }
        ));
    });

    Ok(())
}
