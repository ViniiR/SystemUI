use gtk::glib::{self, g_warning};

#[derive(Debug, Clone)]
pub enum HandlerError<'a> {
    DBusError,
    #[allow(dead_code)]
    ObjectError(&'a str),
}

pub struct Program;
impl Program {
    // Vinii's Graphical System Controller (Daemon)
    pub const PATH: &str = "/com/vinii/vgsc";
    pub const NAME: &str = "com.vinii.vgsc";

    pub const BACKEND_PATH: &str = "/com/vinii/vgsc";
    pub const BACKEND_NAME: &str = "com.vinii.vgsc";
}

// TODO: idk, but make it prettier, on dev side
pub mod dbus {
    pub struct Controllers;
    impl Controllers {
        pub const BRIGHTNESS: &str = "/com/vinii/vgsc/Brightness";
        pub const BATTERY: &str = "/com/vinii/vgsc/Battery";
        pub const POWER: &str = "/com/vinii/vgsc/Power";
        pub const BOOST: &str = "/com/vinii/vgsc/Boost";
        pub const CONSERVATION: &str = "/com/vinii/vgsc/Conservation";
        pub const AUDIO: &str = "/com/vinii/vgsc/Audio";

        pub fn to_interface(interface: &str) -> String {
            interface
                .chars()
                .skip(1)
                .collect::<String>()
                .replace('/', ".")
        }
    }

    pub struct Methods;
    impl Methods {
        pub const SET_BRIGHTNESS: &str = "SetBrightness";
        pub const GET_BRIGHTNESS: &str = "GetBrightness";
        pub const SHUTDOWN: &str = "Shutdown";
        pub const REBOOT: &str = "Reboot";
        pub const LOGOUT: &str = "Logout";
        pub const GET_BATTERY: &str = "GetBattery";
        pub const TOGGLE_BOOST: &str = "ToggleBoost";
        pub const GET_BOOST: &str = "GetBoost";
        pub const TOGGLE_CONSERVATION: &str = "ToggleConservation";
        pub const GET_CONSERVATION: &str = "GetConservation";
        pub const GET_AUDIO: &str = "GetAudio";
        pub const SET_AUDIO: &str = "SetAudio";
        pub const TOGGLE_AUDIO_MUTED: &str = "ToggleAudioMuted";
    }

    pub struct Timeout;
    impl Timeout {
        pub const NONE: i32 = -1;
    }
}

#[derive(Default)]
pub struct State {
    pub volume: u32,
    pub is_muted: bool,
}
impl State {
    pub fn set_volume(&mut self, value: u32) {
        self.volume = value;
    }
    pub fn set_muted(&mut self, value: bool) {
        self.is_muted = value;
    }

    pub fn toggle_muted(&mut self) {
        self.is_muted = !self.is_muted;
    }

    pub fn update(&mut self, variant: glib::Variant) {
        let percentage = variant.child_value(0).get::<u32>();
        let is_muted = variant.child_value(1).get::<bool>();

        match (percentage, is_muted) {
            (Some(percentage), Some(is_muted)) => {
                self.set_volume(percentage);
                self.set_muted(is_muted);
            }
            _ => {
                g_warning!(None, "GetAudio callback returned invalid tuple types");
            }
        }
    }
}
