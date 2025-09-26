package org.pazolli.glypha

import java.util.Random

fun makeRandomName(): String {
    var name: String = ""
    val random = Random()

    val name1 = arrayOf(
        "adorable", "adventurous", "amusing", "beloved", "bubbly", "charming", "cheerful", "chilly", "delightful", "diligent",
        "disguised", "electric", "elegant", "enlightened", "euphoric", "experienced", "fancy", "fitting", "flamboyant", "friendly", "fussy", "glittering",
        "frizzy", "eminent", "fumbling", "fortunate", "fruitful", "giddy", "gleaming", "golden", "grand", "green", "hairy", "haunted", "healthy",
        "humble", "hot", "idle", "joyous", "kooky", "lustrous", "meek", "merry", "nervous", "novel", "oval", "overjoyed", "passionate", "perfect",
        "prime", "puzzled", "quicfunk", "rosy", "rowdy", "royal", "salty", "sane", "serious", "shady", "shallow", "pleasant", "shy", "silent", "striking",
        "singing", "tender", "timid", "timely", "tidy", "trustworthy", "violet", "vivid", "worn", "young", "yummy","zany","zealous", "curious"
    )

    val name2 = arrayOf(
        "rabbit", "turtle", "scarab", "rogue", "beach", "sun", "book", "plane", "mouse", "robin", "cat", "towel", "raquet", "song", "nose",
        "feet", "doctor", "zebra", "box", "bug", "apple", "dingo", "elephant", "fox", "gap", "hope", "llama", "maple", "noun", "octopus",
        "patch", "queen", "rocket", "swan", "umbrella", "view", "wax", "jade"
    )

    do {
        name = name1[random.nextInt(name1.size)].replaceFirstChar { it.uppercase() } + " " + name2[random.nextInt(name2.size)].replaceFirstChar { it.uppercase() }
    } while (name.length > 16)

    return name
}
