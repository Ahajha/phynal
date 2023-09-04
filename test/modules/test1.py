import unittest
from module1 import add, identity, identity_long, nothing


class TestPrimitiveCasters(unittest.TestCase):
    def _check_identity(self, func, value):
        self.assertEqual(func(value), value)

    def test_identity(self):
        for i in range(30):
            self._check_identity(identity, i)

        # Check on edges of input domain
        self._check_identity(identity, 2**31 - 1)
        self._check_identity(identity, -2**31)

        # Too large for int, but small enough for long
        self.assertRaises(OverflowError, identity, 2**31)
        # Too large for long
        self.assertRaises(OverflowError, identity, 2**69)
        # Too small for int, 'large' enough for long
        self.assertRaises(OverflowError, identity, -2**31)

    def test_identity(self):
        for i in range(30):
            self._check_identity(identity, i)

        # Check on edges of input domain
        self._check_identity(identity_long, 2**63 - 1)
        self._check_identity(identity_long, -2**63)

        # Too large for long
        self.assertRaises(OverflowError, identity_long, 2**63)
        # Too small for int, 'large' enough for long
        self.assertRaises(OverflowError, identity_long, -2**63 - 1)

    def test_add(self):
        self.assertEqual(add(3, 4), 7)

    def test_nothing(self):
        self.assertIsNone(nothing(4))
