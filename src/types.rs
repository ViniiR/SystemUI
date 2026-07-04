#[derive(Debug, Clone)]
pub enum HandlerError {
    DBusError,
    ObjectError,
}

pub enum DBusObjects {
    BrightnessController,
    AudioController,
    BatteryController,
    PowerController,
}
impl DBusObjects {
    pub fn as_str(&self) -> &'static str {
        match self {
            Self::BrightnessController => "/com/vinii/BrightnessController",
            Self::AudioController => "/com/vinii/AudioController",
            Self::BatteryController => "/com/vinii/BatteryController",
            Self::PowerController => "/com/vinii/PowerController",
        }
    }
}
