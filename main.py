import matplotlib.pyplot as plt
from collections import Counter

from nltk.sentiment import SentimentIntensityAnalyzer
from nltk.stem import PorterStemmer

from gensim import corpora
from gensim.models import LdaModel
from nltk.tokenize import word_tokenize
from nltk.corpus import stopwords
import re

def load_input_text(file_path):
    with open(file_path, 'r') as file:
        data = file.read().rstrip()
    return data

def load_profanity_words(file_path):
    with open(file_path, 'r', encoding='utf-8') as file:
        profanity_words = set(line.strip() for line in file)
    return profanity_words

def filter_profanity(sentence, profanity_words):
    words = re.findall(r'\b\w+\b', sentence)
    word_counter = Counter(words)
    # Számoljuk meg a trágár szavakat
    profanity_count = sum(word_counter.get(word.lower(), 0) for word in profanity_words)

    return profanity_count > 0

def plot_most_common_words(words, counts):
    if words:
        plt.figure(figsize=(10, 6))
        plt.bar(words, counts, color='coral')
        plt.title('Top 5 leggyakoribb szó')
        plt.xlabel('Szó')
        plt.ylabel('Előfordulások száma')
    else:
        print('Nem található leggyakoribb szó.')


def plot_profanity_counts(profanity_counts):
    plt.figure(figsize=(8, 6))
    plt.bar(profanity_counts.keys(), profanity_counts.values(), color='red')
    plt.title('Trágár szavak előfordulásainak összegzése')
    plt.xlabel('Szó')
    plt.ylabel('Előfordulások száma')


def plot_top_words(words, profanity_words):
    colors = ['red' if word.lower() in profanity_words else 'blue' for word, _ in words]

    plt.figure(figsize=(12, 8))
    plt.bar([word for word, _ in words], [count for _, count in words], color=colors)
    plt.title('Top 10 leggyakoribb szó')
    plt.xlabel('Szó')
    plt.ylabel('Előfordulások száma')
    plt.show()


def analyze_text(text, profanity_words):
    sentences = re.split(r'(?<!\w\.\w.)(?<![A-Z][a-z]\.)(?<=\.|\?)\s', text)  # Mondatok kinyerése

    profanity_count = 0
    profanity_counts = Counter()
    most_common_words = Counter()
    censored_sentences = []

    for sentence in sentences:
        censored_sentence = sentence  # Új változó a csillagozott mondatnak

        # Szűrjük ki a trágár szavakat
        result = filter_profanity(sentence, profanity_words)

        # Trágár szavak számolása
        if result:
            profanity_count += 1

            # Csillagozzuk ki a trágár szavakat
            words = re.findall(r'\b\w+\b', sentence)
            for word in words:
                if word.lower() in profanity_words:
                    censored_sentence = censored_sentence.replace(word, '*' * len(word))
                    profanity_counts[word.lower()] += 1  # Frissítjük a trágár szavak előfordulásának számát

        # Leggyakoribb szók számolása
        most_common_words.update(re.findall(r'\b\w+\b', sentence))

        # Cenzúrázott mondat hozzáadása
        censored_sentences.append(censored_sentence)

    most_common_words = most_common_words.most_common(10)

    return profanity_count, profanity_counts, most_common_words, censored_sentences

def preprocess_text(text):
    stemmer = PorterStemmer()
    tokens = word_tokenize(text)
    print("--------Tokens--------")
    print(tokens)
    print("--------------------------")
    stemmed_tokens = [stemmer.stem(token) for token in tokens]
    print("--------stemmed_tokens--------")
    print(stemmed_tokens)
    print("--------------------------")
    return ' '.join(stemmed_tokens)

def analyze_sentiment(text):
    sid = SentimentIntensityAnalyzer()
    sentiment_scores = sid.polarity_scores(text)
    return sentiment_scores

def interpret_sentiment(compound_score):
    if compound_score >= 0.05:
        return "A szöveg általánosan Pozitív érzelmeket tartalmaz"
    elif -0.05 < compound_score < 0.05:
        return "A szöveg általánosan Semleges érzelmeket tartalmaz"
    else:
        return "A szöveg általánosan Negatív érzelmeket tartalmaz"

def preprocess_text_for_lda(text):
    stop_words = set(stopwords.words("english"))  # Használd a megfelelő nyelvi stopszó listát
    tokens = word_tokenize(re.sub(r'[^\w\s]', '', text.lower()))
    filtered_text = [word for word in tokens if word.isalnum() and word not in stop_words]
    return filtered_text

def main():
    # Input szöveg
    text = load_input_text('data.txt')
    # Preprocessing
    input_text = preprocess_text(text)
    print(input_text)

    # Trágár szavak betöltése
    profanity_words = load_profanity_words('profanity_words.txt')

    # Szöveg elemzése
    profanity_count, profanity_counts, most_common_words, censored_sentences = analyze_text(
        input_text, profanity_words
    )

    # Eredmények kiírása
    print(f"Trágár szavak száma: {profanity_count}")
    print("\nTrágár szavak előfordulásainak összegzése:")
    for word, count in profanity_counts.items():
        print(f"{word}: {count} előfordulás")

    print("\nTop 10 leggyakoribb szavak:")
    for word, count in most_common_words:
        print(f"{word}: {count} előfordulás")

    print("\nCsillagozott mondatok:")
    for censored_sentence in censored_sentences:
        print(censored_sentence)

    # Érzelmi elemzés
    sentiment_scores = analyze_sentiment(input_text)
    compound_score = sentiment_scores['compound']
    sentiment_label = interpret_sentiment(compound_score)

    print("\nÉrzelmi értékelés:")
    print(f"Compound pontszám: {compound_score}")
    print(f"Értelmezés: {sentiment_label}")

    # Vizuális eredmények
    plot_profanity_counts(profanity_counts)
    plot_top_words(most_common_words, profanity_words)
    plt.show()

    ### Kulcsszavak kinyerése
    lda_text = input_text # Preprocesselt text
    filtered_text = preprocess_text_for_lda(lda_text)
    # Szótár létrehozása
    dictionary = corpora.Dictionary([filtered_text])
    # Korpus létrehozása
    corpus = [dictionary.doc2bow(filtered_text)] # bow format: list of (token_id, token_count) tuples.
    # LDA model létrehozása és tanítása
    lda_model = LdaModel(corpus, num_topics=2, id2word=dictionary, passes=10)
    # Kulcsszavak és hozzájuk tartozó súlyok kiírása
    topics = lda_model.print_topics(num_words=5)
    print("\nKulcsszavak és súlyuk a szövegben:")
    for topic in topics:
        print(topic)

if __name__ == "__main__":
    main()