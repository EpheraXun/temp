####################################################################################
Description:
    m1.py, m2.py, m3.py can help you to extract "suite" from your Python project
####################################################################################
Usage:
    1. Choose one of these scripts according to the suite from building configuration of your current project, 
       m1.py for "community", m2.py for "academic", m3.py for "enterprise".
    2. Use the following code to extract "suite":
          from m import C  # m is either m1.py or m2.py or m3.py
          obj = C()
          suite = obj.G()
          """
            Pass suite to verification functions
          """

Note:
    You need to use the maximum optimization level to compile m*.py to pyc files.
