# Written by Wouter Ensink

from core.session import Session


class JsonFileSequenceImporter:
    def __init__(self):
        pass

    def load_sequence(self, file_path) -> Session:
        return Session()
