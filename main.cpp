#include <map>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>

class BadNumberError {};
class BadIndexError {};

const size_t NUMLEN = 4;

class Number {
public:
  explicit Number(const char *num) {
    init(num);
  }

  explicit Number(unsigned int value) {
    char numbuffer[25];
    char nummaskformat[10];
    sprintf(nummaskformat, "%%0%dd", NUMLEN);
    sprintf(numbuffer, nummaskformat, value);
    init(numbuffer);
  }

  Number() {
    char num[NUMLEN + 1];
    for (size_t i = 0; i < NUMLEN; ++i) {
      num[i] = '0' + i;
    }
    num[NUMLEN] = '\0';
    init(num);
  }

  static Number get_random_number() {
    for ( ; ; ) {
      try {
        return Number(rand() % div());
      } catch (BadNumberError) {
        /* ничего не делаем */
      }
    }
  }

  static const std::vector<Number>& all_numbers() {
    static std::vector<Number> s_all_numbers;

    if (s_all_numbers.empty()) {
      const unsigned int total = div();
      s_all_numbers.reserve(total);

      for (unsigned int i = 0; i < total; ++i) {
        try {
          s_all_numbers.push_back(Number(i));
        } catch (BadNumberError) {
          /* ничего не делаем */
        }
      }

      std::vector<Number>(s_all_numbers).swap(s_all_numbers);
    }

    return s_all_numbers;
  }

  char operator[](size_t i) {
    return i < NUMLEN ? m_digits[i] : throw BadIndexError();
  }

  bool operator<(const Number& rhs) const {
    size_t i = 0;
    while (i < NUMLEN && m_digits[i] == rhs.m_digits[i]) {
      ++i;
    }
    return (i < NUMLEN) && (m_digits[i] < rhs.m_digits[i]);
  }

  const char *str(char (&buffer)[NUMLEN + 1]) const {
    for (size_t i = 0; i < NUMLEN; ++i) {
      buffer[i] = m_digits[i];
    }
    buffer[NUMLEN] = '\0';
    return buffer;
  }

private:
  void init(const char *num) {
    if (strlen(num) != NUMLEN) {
      throw BadNumberError();
    }

    for (size_t i = 0; i < NUMLEN; ++i) {
      char digit = num[i];
      for (size_t j = i + 1; j < NUMLEN; ++j) {
        if (num[j] == digit) {
          throw BadNumberError();
        }
      }
      m_digits[i] = digit;
    }
  }

  static unsigned int div() {
    unsigned int result = 1;
    for (size_t i = 0; i < NUMLEN; ++i) {
      result *= 10;
    }
    return result;
  }

  char m_digits[NUMLEN];
};

class Match {
public:
  Match(Number x, Number y)
    : oxes(0), cows(0)
  {
    for (size_t i = 0; i < NUMLEN; ++i) {
      oxes += (x[i] == y[i]);
    }

    for (size_t i = 0; i < NUMLEN; ++i) {
      for (size_t j = 0; j < NUMLEN; ++j) {
        cows += (x[i] == y[j] && i != j);
      }
    }
  }

  Match(int oxes, int cows)
    : oxes(oxes), cows(cows)
  {
    /* пусто */
  }

  Match()
    : oxes(0), cows(0)
  {
    /* пусто */
  }

  bool operator<(const Match& rhs) const {
    return (oxes < rhs.oxes) || ((oxes == rhs.oxes) && (cows < rhs.cows));
  }

  const char *str(char buffer[]) const {
    sprintf(buffer, "%dA%dB", oxes, cows);
    return buffer;
  }

private:
  int oxes, cows;
};

struct Answer {
  Number number;
  Match match;

  Answer(Number number, Match match)
    : number(number), match(match)
  {
    /* пусто */
  }

  void print() {
    char num_buffer[NUMLEN + 1];
    char match_buffer[10];
    printf("%s<->%s", number.str(num_buffer), match.str(match_buffer));
  }
};

class AnswerSource {
public:
  virtual ~AnswerSource() {}
  virtual Answer nextAnswer(Number num) = 0;
};

class UserAnswerSource: public AnswerSource {
public:
  virtual Answer nextAnswer(Number num) {
    char buffer[NUMLEN + 1];
    printf("number is %s\n", num.str(buffer));
    printf("type oxes, cows >");
    int oxes, cows;
    scanf("%d %d", &oxes, &cows);
    return Answer(num, Match(oxes, cows));
  }
};

class BotAnswerSource: public AnswerSource {
public:
  BotAnswerSource(Number known)
    :m_known(known)
  {
    /* пусто */
  }

  virtual Answer nextAnswer(Number num) {
    return Answer(num, Match(m_known, num));
  }

private:
  Number m_known;
};

//

int main() {
  try {
    typedef std::vector<Number> Numbers;
    typedef std::map<Match, Numbers> Classes;
    Classes classes;
    Number sample(1234);
    unsigned int total = 0;
    for (
      Numbers::const_iterator p = Number::all_numbers().begin();
      p != Number::all_numbers().end();
      ++p
    ) {
      classes[Match(sample, *p)].push_back(*p);
      ++total;
    }

    double info = 0;
    for (Classes::const_iterator p = classes.begin(); p != classes.end(); ++p) {
      char match_buf[10];
      double count = p->second.size();
      info -= count / total * log(count / total) / log(2);
      printf("class %s <--> %f\n", p->first.str(match_buf), count);
    }
    printf("info = %f\n", info);
  } catch (BadNumberError) {
    printf("uncatched BadNumberError\n");
  } catch (BadIndexError) {
    printf("uncatched BadIndexError\n");
  } catch (...) {
    printf("uncatched unknown error\n");
  }
  return 0;
}
