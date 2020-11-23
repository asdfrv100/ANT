module.exports = {
  'env': {
    'browser': true,
    'commonjs': true,
    'es2021': true,
  },
  'extends': [
    'google',
  ],
  'parserOptions': {
    'ecmaVersion': 12,
  },
  'rules': {
    'no-var': 0,
    'indent': ['error', 2],
    'comma-dangle': 0,
    'space-before-function-paren': 0
  },
};
