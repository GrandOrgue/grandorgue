#!/usr/bin/env python3

# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
# License GPL-2.0 or later
# (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).

"""Consistency checker for GrandOrgue .po translation files.

gettext itself only validates that a catalogue compiles. It does not notice
that a translator dropped a menu accelerator, a mnemonic marker, or the
leading space of a string that gets concatenated at run time. Those defects
break the UI without producing a single msgfmt warning, so they are checked
here instead.

Usage:
    python3 build-scripts/check-po.py po/*.po
    python3 build-scripts/check-po.py --warnings po/ru.po

Exits with status 1 if any error was reported. Warnings never affect the
exit status.
"""

import argparse
import re
import sys

# printf conversion specification, e.g. %s, %d, %.3f, %02d, %u, %%
FORMAT_SPEC_RE = re.compile(r"%[-+ #0]*[0-9*]*(?:\.[0-9*]+)?[hlLqjzt]*[a-zA-Z%]")

# One "msgid"/"msgstr" line plus its continuation lines. A long entry is split
# over several physical lines, each holding its own quoted fragment, so the
# separator between fragments may be a newline and not only a space.
ENTRY_RE = re.compile(r'^(msgid|msgstr)((?:[ \t\n]*"(?:[^"\\]|\\.)*")+)', re.M)

STRING_RE = re.compile(r'"((?:[^"\\]|\\.)*)"')

# Technical names that must remain byte-for-byte recognisable in translations.
PROTECTED_TERMS = (
    "ALSA",
    "ASIO",
    "BPM",
    "CMB",
    "CRC",
    "GrandOrgue",
    "IEEE",
    "JACK",
    "MIDI",
    "MIDIInputNumber",
    "NRPN",
    "ODF",
    "PA",
    "PCM",
    "RPN",
    "ReferencePipe",
    "SYSEX",
    "SetterElement",
    "WAV",
)

# A final full stop is part of these short on/off abbreviations rather than
# sentence punctuation.
FINAL_PERIOD_ABBREVIATION_RE = re.compile(
    r"(?:вкл|выкл|увімк|вимк|укл|įj|išj)\.$", re.IGNORECASE
)

# These gettext entries are intentionally left in English for project-specific
# reasons and must not be localised even though most ordinary labels are.
UNCHANGED_ENTRIES = {"Settings"}

# Escapes that gettext writes into a .po file, mapped to what they mean
ESCAPES = {
    "\\n": "\n",
    "\\t": "\t",
    "\\r": "\r",
    "\\\\": "\\",
    '\\"': '"',
}


def unescape(s):
    """Convert the escaped form stored in a .po file into the actual text."""
    out = []
    i = 0
    n = len(s)

    while i < n:
        pair = s[i : i + 2]

        if pair in ESCAPES:
            out.append(ESCAPES[pair])
            i += 2
        else:
            out.append(s[i])
            i += 1

    return "".join(out)


def parse_po(path):
    """Return the list of (lineNo, msgid, msgstr) of a .po file.

    Header, obsolete and plural entries are skipped: the header has an empty
    msgid, and plural entries are checked by msgfmt already.
    """
    with open(path, encoding="utf-8") as f:
        text = f.read()

    entries = []
    lineStarts = [0]

    for m in re.finditer("\n", text):
        lineStarts.append(m.end())

    def lineOf(pos):
        lo, hi = 0, len(lineStarts) - 1

        while lo < hi:
            mid = (lo + hi + 1) // 2
            if lineStarts[mid] <= pos:
                lo = mid
            else:
                hi = mid - 1

        return lo + 1

    pending = None

    for m in ENTRY_RE.finditer(text):
        keyword = m.group(1)
        value = unescape("".join(STRING_RE.findall(m.group(2))))

        if keyword == "msgid":
            pending = (lineOf(m.start()), value)
        elif pending:
            msgLine, msgId = pending
            pending = None

            if msgId:
                entries.append((msgLine, msgId, value))

    return entries


def edge_spaces(s):
    """Return the leading and the trailing whitespace of a string."""
    return (s[: len(s) - len(s.lstrip())], s[len(s.rstrip()) :])


def check_entry(msgId, msgStr, isCheckingTerminology):
    """Return the lists of errors and of warnings found in a single entry."""
    errors = []
    warnings = []

    if msgStr:
        idSpecs = FORMAT_SPEC_RE.findall(msgId)
        strSpecs = FORMAT_SPEC_RE.findall(msgStr)

        if sorted(idSpecs) != sorted(strSpecs):
            errors.append(
                "format specifications differ: %s vs %s"
                % (idSpecs or "none", strSpecs or "none")
            )

        for name, ch in (("tab", "\t"), ("newline", "\n")):
            nId = msgId.count(ch)
            nStr = msgStr.count(ch)

            if nId != nStr:
                errors.append("%s count differs: %d vs %d" % (name, nId, nStr))

        idLead, idTrail = edge_spaces(msgId)
        strLead, strTrail = edge_spaces(msgStr)

        if idLead != strLead:
            errors.append(
                "leading whitespace differs: %r vs %r" % (idLead, strLead)
            )

        if idTrail != strTrail:
            errors.append(
                "trailing whitespace differs: %r vs %r" % (idTrail, strTrail)
            )

        if "&" in msgId and "&" not in msgStr:
            errors.append("mnemonic '&' is missing")
        elif "&" not in msgId and "&" in msgStr:
            errors.append("mnemonic '&' was added")
        elif "&" in msgId and "&" in msgStr:
            mnemonic = mnemonic_of(msgStr)

            if mnemonic and mnemonic not in msgStr.replace("&", "", 1).lower():
                errors.append(
                    "mnemonic '&%s' does not occur in the translation" % mnemonic
                )

        if (
            msgId == msgStr
            and msgId not in UNCHANGED_ENTRIES
            and re.search(r"[A-Za-z]{3}", msgId)
        ):
            warnings.append("left untranslated")

        if isCheckingTerminology:
            if msgId in UNCHANGED_ENTRIES and msgStr != msgId:
                errors.append("entry must remain untranslated")

            for term in PROTECTED_TERMS:
                if re.search(
                    r"(?<![A-Za-z])%s(?![A-Za-z])" % re.escape(term), msgId
                ):
                    if not re.search(
                        r"(?<![A-Za-z])%s(?![A-Za-z])" % re.escape(term), msgStr
                    ):
                        errors.append("protected term %r is missing or altered" % term)

        if (
            len(msgId) > 12
            and not msgId.endswith(".")
            and msgStr.endswith(".")
            and not FINAL_PERIOD_ABBREVIATION_RE.search(msgStr)
        ):
            warnings.append("final period added that the original does not have")

    return errors, warnings


def mnemonic_of(s):
    """Return the lowercased letter marked by '&', or None."""
    m = re.search(r"&(.)", s)

    return m.group(1).lower() if m and m.group(1) != "&" else None


def check_mnemonic_conflicts(entries):
    """Return the mnemonics used by more than one translated label.

    Menus are not identifiable from a .po file, so this cannot tell a real
    conflict from two labels living in different menus. It is reported as a
    warning only, to be reviewed by hand.
    """
    byMnemonic = {}

    for _, msgId, msgStr in entries:
        if "&" in msgId and "&" in msgStr:
            mnemonic = mnemonic_of(msgStr)

            if mnemonic:
                byMnemonic.setdefault(mnemonic, []).append(msgStr)

    return {k: v for k, v in byMnemonic.items() if len(v) > 1}


def check_file(path, isShowingWarnings, isShowingConflicts):
    """Check one .po file and print its findings. Return the error count."""
    entries = parse_po(path)
    isCheckingTerminology = re.search(r"(?:^|/)(?:be|lt|ru|uk)\.po$", path) is not None
    nErrors = 0
    nWarnings = 0

    for lineNo, msgId, msgStr in entries:
        errors, warnings = check_entry(msgId, msgStr, isCheckingTerminology)

        for text in errors:
            print("%s:%d: error: %s" % (path, lineNo, text))
            print("    msgid  %r" % msgId)
            print("    msgstr %r" % msgStr)

        nErrors += len(errors)

        if isShowingWarnings:
            for text in warnings:
                print("%s:%d: warning: %s: %r" % (path, lineNo, text, msgId))

        nWarnings += len(warnings)

    if isShowingConflicts:
        for mnemonic, labels in sorted(check_mnemonic_conflicts(entries).items()):
            print(
                "%s: warning: mnemonic '&%s' is shared by %d labels: %s"
                % (path, mnemonic, len(labels), ", ".join(sorted(labels)))
            )

    print(
        "%s: %d entries, %d errors, %d warnings"
        % (path, len(entries), nErrors, nWarnings)
    )

    return nErrors


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("files", nargs="+", metavar="FILE.po")
    parser.add_argument(
        "-w",
        "--warnings",
        action="store_true",
        help="also list the entries left untranslated",
    )
    parser.add_argument(
        "-m",
        "--mnemonic-conflicts",
        action="store_true",
        help="also list the mnemonic letters shared by several labels",
    )
    args = parser.parse_args()

    nErrors = 0

    for path in args.files:
        nErrors += check_file(path, args.warnings, args.mnemonic_conflicts)

    return 1 if nErrors else 0


if __name__ == "__main__":
    sys.exit(main())
