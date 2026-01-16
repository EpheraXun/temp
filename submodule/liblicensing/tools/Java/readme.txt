####################################################################################
Description:
    M1.java, M2.java, M3.java can help you to extract "suite" from your Java project
####################################################################################
Usage:
    1. Choose one of these classes according to the suite from building configuration of your current project,
       M1 for "community", M2 for "academic", M3 for "enterprise".
    2. Use the following code to extract "suite":
          import dhl.M   // M is either M1 or M2 or M3
          obj = new M1()
          suite = obj.G(123, 456, 789) // You can input random number of int args here
          """
            Pass suite to verification functions
          """

