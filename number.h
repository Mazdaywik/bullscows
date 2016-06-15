#ifndef NUMBER_H_
#define NUMBER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

class BadNumberError {};
class BadIndexError {};

const size_t NUMLEN = 4;

class Number;
typedef std::vector<Number> Numbers;

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

  static const Numbers& all_numbers() {
    static Numbers s_all_numbers;

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

      Numbers(s_all_numbers).swap(s_all_numbers);
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

#endif // NUMBER_H_
