import mutagen
import magic # For MIME type detection
from analyzer import Analyzer

class MetadataAnalyzer(Analyzer):

    def __init__(self):
        pass

    @staticmethod
    def analyze(filename):

        metadata = dict()
        #Extract metadata from an audio file using mutagen
        audio_file = mutagen.File(filename, easy=True)

        #Grab other file information that isn't encoded in a tag, but instead usually
        #in the file header. Mutagen breaks that out into a separate "info" object:
        info = audio_file.info
        metadata["sample_rate"] = info.sample_rate
        metadata["length_seconds"] = info.length
        metadata["bit_rate"] = info.bitrate
        #metadata["channels"] = info.channels
      
        #Use the python-magic module to get the MIME type.
        mime_magic = magic.Magic(mime=True)
        metadata["mime_type"] = mime_magic.from_file(filename)

        #Try to get the number of channels if mutagen can...
        try:
            #Special handling for getting the # of channels from MP3s. It's in the "mode" field
            #which is 0=Stereo, 1=Joint Stereo, 2=Dual Channel, 3=Mono. Part of the ID3 spec...
            if metadata["mime_type"] == "audio/mpeg":
                if info.mode == 3:
                    metadata["channels"] = 1
                else:
                    metadata["channels"] = 2
            else:
                metadata["channels"] = info.channels
        except (AttributeError, KeyError):
            #If mutagen can't figure out the number of channels, we'll just leave it out...
            pass

        #We normalize the mutagen tags slightly here, so in case mutagen changes,
        #we find the 
        mutagen_to_airtime_mapping = {
            'title':        'track_title',
            'artist':       'artist_name',
            'album':        'album_title',
            'bpm':          'bpm',
            'composer':     'composer',
            'conductor':    'conductor',
            'copyright':    'copyright',
            'encoded_by':   'encoder',
            'genre':        'genre',
            'isrc':         'isrc',
            'label':        'label',
            'language':     'language',
            'last_modified':'last_modified',
            'mood':         'mood',
            'replay_gain':  'replaygain',
            'track_number': 'track_number',
            'track_total':  'track_total',
            'website':      'website',
            'year':         'year',
            'mime_type':    'mime',
        }

        for mutagen_tag, airtime_tag in mutagen_to_airtime_mapping.iteritems():
            try:
                metadata[airtime_tag] = audio_file[mutagen_tag]

                # Some tags are returned as lists because there could be multiple values.
                # This is unusual so we're going to always just take the first item in the list.
                if isinstance(metadata[airtime_tag], list):
                    metadata[airtime_tag] = metadata[airtime_tag][0]

            except KeyError:
                pass

        return metadata



'''
For reference, the Airtime metadata fields are:
            title
            artist ("Creator" in Airtime)
            album
            bit rate
            BPM
            composer
            conductor
            copyright
            cue in
            cue out
            encoded by
            genre
            ISRC
            label
            language
            last modified
            length
            mime
            mood
            owner
            replay gain
            sample rate
            track number
            website
            year
'''
