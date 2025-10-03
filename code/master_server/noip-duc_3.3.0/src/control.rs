use std::sync::mpsc;
use std::thread;
use std::time::Duration;

pub enum Control {
    UpdateNow,
    NotifyNextCheck,
    Quit,
}

pub trait Controller {
    fn recv_timeout(&self, timeout: Duration) -> Option<Control>;
}

pub struct SleepOnlyController;

impl Controller for SleepOnlyController {
    fn recv_timeout(&self, timeout: Duration) -> Option<Control> {
        thread::sleep(timeout);
        Some(Control::UpdateNow)
    }
}

impl Controller for mpsc::Receiver<Control> {
    fn recv_timeout(&self, timeout: Duration) -> Option<Control> {
        self.recv_timeout(timeout).ok()
    }
}
