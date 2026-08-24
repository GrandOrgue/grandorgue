<!--
Copyright 2006 Milan Digital Audio LLC
Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
License GPL-2.0 or later
(https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
-->

# Translation glossary

GrandOrgue's interface is full of pipe organ terms that look like ordinary
English words. A translator — human or machine — who renders them literally
produces text that is grammatically correct and completely wrong: `Stop`
becomes a verb, `Rank` becomes a rating, `Piston` becomes an engine part,
`General` becomes a military officer.

This file records the agreed rendering of those terms, so that every
catalogue uses the same word for the same concept and new translations do not
have to rediscover the traps.

## How to use it

- When a term below appears in a `msgid`, use the listed translation.
- Keep the term identical across the whole catalogue. Two spellings of
  `Coupler` in one file is a defect even if both are defensible.
- A term missing here is not covered by any decision — translate it and add
  a row, rather than inventing a second convention silently.
- `de.po` is the oldest proofread catalogue and a useful second opinion on
  organ vocabulary.

Before committing a catalogue, run:

```
python3 build-scripts/check-po.py po/*.po
```

It catches the structural defects a glossary cannot: lost `\t` accelerators,
lost `&` mnemonics, lost edge whitespace, and mismatched printf formats.

## Organ terms

| English | de (reference) | ru | uk | be | lt |
|---|---|---|---|---|---|
| Stop | Stopp | Регистр | Регістр | Рэгістр | Registras |
| Rank | Reihe | Ранк | Ранк | Ранк | Rankas |
| Pipe | Pfeife | Труба | Труба | Труба | Vamzdis |
| Manual | Manual | Мануал | Мануал | Мануал | Manualas |
| Coupler | Koppel | Копула | Копула | Счэпка | Kopula |
| Tremulant | Tremulant | Тремулянт | Тремулянт | Трэмулянт | Tremulantas |
| Enclosure | Schweller | Швеллер | Швелер | Швелер | Šveleris |
| Windchest | Windlade | Виндлада | Віндлада | Віндлада | Oro dėžė |
| Piston | Piston | Пистон | Пістон | Пістон | Pistonas |
| Setter | Setzer | Секвенсор | Секвенсор | Секвенсар | Sekvenceris |
| Sequencer | Sequenzer | Секвенсор | Секвенсор | Секвенсар | Sekvenceris |
| Combination | Kombination | Комбинация | Комбінація | Камбінацыя | Derinys |
| General (combination) | General-Setzer | Общая | Загальна | Агульная | Bendroji |
| Division (organ section/work) | Werk | секция | секція | секцыя | sekcija |
| Divisional (combination) | Einzelwerk-Setzer | Секционная комбинация | Секційна комбінація | Секцыйная камбінацыя | Sekcijos derinys |
| Divisional Coupler | Koppeln | Секционная копула | Секційна копула | Секцыйная счэпка | Sekcijų kopula |
| Crescendo | Crescendo | Крещендо | Крещендо | Крэшчэнда | Krescendo |
| Temperament | Stimmung | Темперация | Темперація | Тэмперацыя | Derinimo sistema |
| Wolf (fifth) | Wolf | волк | вовк | воўк | vilkas |
| Meantone | mitteltönig | среднетоновый | середньотоновий | сярэднетонавы | vidutinio tono |
| Tracker delay | Trakturverzögerung | задержка трактуры | затримка трактури | затрымка трактуры | traktūros delsa |
| Comma | Komma | комма | кома | кома | koma |
| Cent | Cent | цент | цент | цэнт | centas |
| pipe organ | Orgel | духовой орган | трубний орган | духавы арган | **vargonai** |
| organ (the instrument) | Orgel | орган | орган | **арган** | **vargonai** |

Two of these are bolded because the obvious cognate is a different word:

- Lithuanian `organas` is a body organ. The instrument is always `vargonai`,
  a plural-only noun, so it never takes a singular form.
- Belarusian `орган` is a body organ or an organ of government. The
  instrument is `арган`.

Machine translation gets both wrong by default, and gets them wrong in only
*some* of the occurrences, which leaves the catalogue looking inconsistent
rather than plainly broken. Grep for the wrong form after any bulk edit.

`sample` also has two senses that English spells the same way: a recorded
pipe sound, and one PCM value in a buffer. Following `de.po` (`Samples pro
Puffer`, `Samplerate`), both use the same word here — do not reach for
"specimen"/"example" words (`образец`, `зразок`, `узор`, `pavyzdys`,
`mėginys`), which is what machine translation produces.

## MIDI and audio terms

| English | ru | uk | be | lt |
|---|---|---|---|---|
| Key (of a keyboard) | клавиша | клавіша | клавіша | klavišas |
| Note | нота | нота | нота | nata |
| Velocity | сила нажатия | сила натискання | сіла націску | paspaudimo jėga |
| Bank (MIDI bank) | банк | банк | банк | bankas |
| Controller | контроллер | контролер | кантролер | valdiklis |
| Channel | канал | канал | канал | kanalas |
| Sample | семпл | семпл | сэмпл | semplas |
| Sample set | набор семплов | набір семплів | набор сэмплаў | semplų rinkinys |
| Samples per buffer | семплов на буфер | семплів на буфер | сэмплаў на буфер | semplų buferyje |
| Bits per sample | бит на семпл | біт на семпл | біт на сэмпл | bitų viename semple |
| Sample rate | частота дискретизации | частота дискретизації | частата дыскрэтызацыі | diskretizavimo dažnis |
| Attack | атака | атака | атака | ataka |
| Release | затухание | згасання | згасанне | atslūgimas |
| Loop | петля | петля | пятля | kilpa |
| Polyphony | полифония | поліфонія | поліфанія | polifonija |
| Reverb | реверберация | реверберація | рэверберацыя | aidėjimas |
| Convolution reverb | свёрточная реверберация | згорткова реверберація | згорткавая рэверберацыя | konvoliucinė reverberacija |
| Bit | бит | біт | біт | bitas |
| Buffer | буфер | буфер | буфер | buferis |
| Cache | кэш | кеш | кэш | talpykla |
| Memory pool | пул памяти | пул пам'яті | пул памяці | atminties telkinys |
| Path (to a file) | путь | шлях | шлях | kelias |
| Impulse response | импульсная характеристика | імпульсна характеристика | імпульсная характарыстыка | impulsinė charakteristika |
| Chunk (file-format block) | фрагмент | фрагмент | фрагмент | fragmentas |
| File signature (`magic`) | сигнатура файла | сигнатура файла | сігнатура файла | failo signatūra |
| Track (MIDI) | дорожка | доріжка | дарожка | takelis |
| Mapping | сопоставление | зіставлення | супастаўленне | susiejimas |
| Latency / delay | задержка | затримка | затрымка | delsa |

## Interface terms

| English | ru | uk | be | lt |
|---|---|---|---|---|
| Value | Значение | Значення | Значэнне | Vertė |
| Volume | Громкость | Гучність | Гучнасць | Garsumas |
| Label | Надпись | Напис | Надпіс | Etiketė |
| Switch (noun) | Переключатель | Перемикач | Пераключальнік | Jungiklis |
| Shortcut (key) | Быстрая клавиша | Швидка клавіша | Хуткая клавіша | Spartusis klavišas |
| Sound engine | Звуковой движок | Звуковий рушій | Гукавы рухавік | Garso variklis |
| Preset | Пресет | Пресет | Прэсет | Presetas |
| Drawstop | Регистровая рукоятка | Регістрова рукоятка | Рэгістровая ручка | Registro rankenėlė |
| Package | Пакет | Пакет | Пакет | Paketas |
| Directory | Каталог | Каталог | Каталог | Katalogas |
| Default | По умолчанию | За замовчуванням | Па змаўчанні | Numatytasis |
| Invalid | Недопустимый | Неприпустимий | Недапушчальны | Netinkamas |
| Toggle | Переключение | Перемикання | Пераключэнне | Perjungimas |
| On value / Off value | Значение вкл. / выкл. | Значення увімк. / вимк. | Значэнне укл. / выкл. | Įjungimo / išjungimo vertė |

## Player and metronome panel buttons

These are drawn on an organ panel, so they must stay short. They are verbs of
media playback, not of games: `PLAY` means "play back a recording", never
"play a game".

| English | ru | uk | be | lt |
|---|---|---|---|---|
| PLAY | ВОСПР. | ВІДТВ. | ПРАЙГР. | GROTI |
| STOP | СТОП | СТОП | СТОП | STOP |
| PAUSE | ПАУЗА | ПАУЗА | ПАЎЗА | PAUZĖ |
| REC | ЗАП | ЗАП | ЗАП | REC |
| REC File | ЗАП в файл | ЗАП у файл | ЗАП у файл | REC į failą |
| ON (metronome) | ВКЛ | УВІМК | ВКЛ | ĮJ |

`MEL`, `BAS`, `BPM`, `G.C.` are fixed abbreviations printed on organ consoles.
Leave them in Latin script in every language.

## Traps

Terms that machine translation reliably gets wrong. Check these explicitly.

| msgid | Wrong because | Correct reading |
|---|---|---|
| `Stop` | read as the verb "to stop" | a register of the organ |
| `Rank` | read as "rating" or "to classify" | a row of pipes of one timbre |
| `Piston` | read as an engine part | a combination button |
| `General` | read as a military rank | a combination affecting the whole organ |
| `Setter` | transliterated into a dog breed | the combination sequencer |
| `Temperament` | read as a character trait | a tuning system |
| `Upper bank:` / `L&ower bank:` | read as a river bank | the MIDI bank number |
| `&MIDI-note:` | read as an annotation | a MIDI note |
| `Highest key:` | read as a cryptographic key | the highest keyboard key |
| `On value:` / `Off value:` | `On`/`Off` read as prepositions | the value meaning on / off |
| `&Memory Set\tShift` | `Shift` read as the word "shift" | an accelerator key name |
| `&Panic\tEscape` | `Escape` read as the word "escape" | an accelerator key name |
| `Export &Combinations` | `&` read as the conjunction "and" | a mnemonic marker |
| `Bermudo (1555)` | read as the Bermuda Islands | Juan Bermudo, music theorist |
| `1/6-comma (dieses)` | read as forms of "to die" | the diesis, a microtonal interval |
| `Beat` | read as the verb "to beat" | a beat of the metronome |
| `PLAY` | read as "play a game" (ГУЛЬНЯ, ŽAISTI) | play back a recording |
| `PAUSE` | read as a written note (lt PASTABA) | pause playback |
| `ON` | read as the preposition "on" (ru НА) | the metronome is on |
| `Windchest %d` | read as a chest or a wind cover | the windchest |
| `A`…`Z`, `F1`…`F24` | transliterated into Cyrillic | labels of physical keyboard keys — never translate |
| `well-temperament` | `well` read as a water well | a well temperament |
| `Memory pool` | `pool` read as a swimming pool (lt baseinas) | a memory pool |
| `comma` (temperament) | read as the punctuation mark | the syntonic comma |
| `fifths` | read as "one fifth parts" | the interval of a fifth |
| `sample` | read as a specimen or an example | a recording, or a PCM value |
| `Sample information` | read as "an example of information" | information about the sample |
| `wave file` | read as a physical wave (lt `banginis` = whale) | a WAV file |
| `Mono` | expanded into "mononucleosis" | the mono channel mode |
| `Disabled` | read as a disabled person | the feature is off |
| `Custom` | read as a custom or tradition | user-defined |
| `Event-&No` | `No` read as the word "no" | `No` is short for "number" |
| `Heun (Jan) 1805` | `Jan` read as January | Jan Heun, a person |
| `manuals, stops` | read as instruction manuals and bus stops | organ manuals and stops |
| `Division`, `Divisional` | read as administrative subdivision, arithmetic division, or an adjective without a noun | an organ section/work, or a combination local to one section |
| `Send Division Output` | read as sending a subdivision or division operation | send the MIDI output of the organ section/manual |
| `bad magic` | translated literally as magic or sorcery | an invalid file-format signature |
| `Release` | read as a software release/version | the release segment or decay of a pipe sample |
| `Update checker` | read as a person or a chess piece | the feature that checks for software updates |
| `trigger` | read as a firearm trigger | the button action that calls `Push()` |
| `select`, `execute`, `print`, `snapshot`, `multiply`, `add`, `subtract`, `divide`, `scroll` | read as UI actions | labels of physical keyboard keys in `GOKeyConvert::SHORTCUTS` |

All strings in `GOKeyConvert::SHORTCUTS` name physical keyboard keys, even when
an English key label looks like a verb. Keep established key names distinct:
`Print` and `Print Screen`, `Windows (left)` and `Windows (right)`, the numeric
keypad digits and operations, and `Scroll Lock`. Latin letter keys, function
keys, and technical labels such as `Num +` must remain in Latin script.

In the Russian, Ukrainian, Belarusian, and Lithuanian catalogues reviewed here,
`Settings` deliberately remains in English in every context, including ordinary
UI labels and the directory name.

## Things never to translate

`MIDI`, `PCM`, `WAV`, `BPM`, `RPN`, `NRPN`, `SYSEX`, `ASIO`, `JACK`, `ALSA`,
`CRC`, `IEEE`, `ODF`, `CMB`, `PA`, `GrandOrgue`, ODF identifiers such as
`ReferencePipe`, `SetterElement`, and `MIDIInputNumber`,
file masks such as `*.wav`, format strings such as `%Y-%m-%d-%H-%M-%S.%l.mid`,
and the accelerator names after a `\t` (`Ctrl+O`, `Shift`, `Escape`, `F1`).

Transliterating an acronym into Cyrillic (`РПН` for `RPN`, `ПКМ` for `PCM`,
`МИДИ` for `MIDI`) makes it unrecognisable — leave it in Latin script.

The language-recovery controls `&Settings...`, `&Language (need to restart)`,
and `Default (...)` also deliberately remain in English. A user who accidentally
selects an unknown language must be able to find the language selector without
understanding the current translation. These labels use `wxT`, not gettext, in
the source code and must not be added to the PO catalogues.
