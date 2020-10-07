# Written by Wouter Ensink

# this class should be the only one actually in charge of editing the session
# this way the editing user interface only needs a reference to this class
# keeps all the details of the engine out of the way

from core.session import Session


class SessionEditor:
    def __init__(self):
        self.session = None

    def set_session(self, session: Session):
        self.session = session
